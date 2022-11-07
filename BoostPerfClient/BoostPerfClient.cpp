#include "BoostPerfClient.h"
#include <iostream>
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

BoostPerfClient::BoostPerfClient(const std::string& hostname, int port, size_t numOfStreams) : m_port{ port }, m_numOfStreams{ numOfStreams }, m_resolver{ make_strand(m_ioc) },
m_endpoint{ m_resolver.resolve(hostname, std::to_string(m_port)) }, m_tempSocket{ m_ioc }
{
    m_sockets.reserve(m_numOfStreams);
    m_runClient = true;
    do_connect();
}
void BoostPerfClient::do_connect()
{
	//this->m_tempSocket.m_socket.async_connect(m_endpoint, [this](boost::system::error_code ec) {});
	boost::asio::async_connect(m_tempSocket.m_socket, m_endpoint, [this](boost::system::error_code ec, const tcp::endpoint& endpoint)
		{
			if (!ec)
			{
				std::cout << "adding endpoint: " << endpoint << '\n';
				m_tempSocket.m_connected = true;
				addSocket();
			}
			else
			{
				m_tempSocket.m_socket.cancel();
				m_tempSocket.m_socket.close();
				m_tempSocket.m_socket = boost::asio::ip::tcp::socket(m_ioc); // resets the socket
				std::cout << "Failed to connect to: " << m_endpoint.begin()->endpoint() << '\n';
				//throw std::runtime_error("Failed to connect to endpoint");
			}
		});
}
void BoostPerfClient::addSocket()
{
	if (m_sockets.size() < m_numOfStreams)
	{
		std::lock_guard<std::mutex> lg(m_socketsMtx);
		m_sockets.emplace_back(std::move(m_tempSocket));
		this->do_connect();
		m_dispatch_cv.notify_one();
	}
}
void BoostPerfClient::runSocketsLoop()
{
	while (true)
	{
		std::unique_lock<std::mutex> lk(m_socketsMtx);
		m_dispatch_cv.wait(lk);
		std::cout << "starting for, size of vec is: " << m_sockets.size() << '\n';
		dispatchSockets();
		cleanupClosedSockets();
	}
}
void BoostPerfClient::runIoContext()
{
	while (m_runClient)
	{
		m_ioc.run();
	}
}


void BoostPerfClient::dispatchSockets()
{
	for (auto& wrappedSocket : m_sockets)
	{
		if (!wrappedSocket.m_dispatched)
		{
			std::cout << "remote endpoint: " << wrappedSocket.m_socket.remote_endpoint() << '\n';
			std::cout << "Am I connected? " << wrappedSocket.m_connected << '\n';
			std::cout << "Am I dispatched? " << wrappedSocket.m_dispatched << '\n';
			wrappedSocket.m_dispatched = true;
			wrappedSocket.dispatchSocketForWrite();
			//wrappedSocket.dispatchSocketForListen();
		}
	}
}

void BoostPerfClient::cleanupClosedSockets()
{
	m_sockets.erase(std::remove_if(m_sockets.begin(), m_sockets.end(), [](const WrappedSocket& ws)-> bool
		{
			std::cout << "on erase lamb\n";
			return ((ws.m_connected == false) && (ws.m_dispatched == true));
		}), m_sockets.end());
}

