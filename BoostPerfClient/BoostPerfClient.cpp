#include "BoostPerfClient.h"
#include "BytesConversion.h"
#include "Timer.h"
#include <chrono>
#include <iostream>

using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

BoostPerfClient::BoostPerfClient(const std::string& hostname, int port, size_t numOfStreams) : m_port{ port }, m_numOfStreams{ numOfStreams }, m_resolver{ m_ioc },
m_endpoint{ m_resolver.resolve(hostname, std::to_string(m_port)) }, m_tempSocket{ std::make_shared<WrappedSocket>(m_ioc) }
{
    m_sockets.reserve(m_numOfStreams);
    m_runClient = true;
    doConnect();
}

void BoostPerfClient::doConnect() // connects a temporary socket to the endpoint
{
	boost::asio::async_connect(m_tempSocket.get()->m_socket, m_endpoint, [this](boost::system::error_code ec, const tcp::endpoint& endpoint)
		{
			if (!ec)
			{
				std::cout << "adding endpoint: " << endpoint << '\n';
				m_tempSocket.get()->m_connected = true;
				addSocket(); // call to move the temp socket to the sockets vector
			}
			else
			{
				m_tempSocket.get()->m_socket.cancel();
				m_tempSocket.get()->m_socket.close();
				m_tempSocket.get()->m_socket = boost::asio::ip::tcp::socket(m_ioc); // resets the socket
				std::cout << "Failed to connect to: " << m_endpoint.begin()->endpoint() << '\n';
			}
		});
}
//called when the temporary socket is connected
void BoostPerfClient::addSocket()
{
	if (m_sockets.size() < m_numOfStreams)
	{
		std::lock_guard<std::mutex> lg(m_socketsMtx);
		m_sockets.emplace_back(m_tempSocket);
		m_tempSocket = std::make_shared<WrappedSocket>(m_ioc); // reset the temp socket
		this->doConnect();
		m_dispatch_cv.notify_one(); // wake up the runSocketsLoop to dispatch the socket to start transactions
	}
}


void BoostPerfClient::runSocketsLoop() // called when new sockets are added, dispatch new sockets and erase closed sockets from the vector.
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
void BoostPerfClient::printSocketsStats()
{
	std::chrono::time_point<std::chrono::system_clock> last =
		std::chrono::system_clock::now();

	std::chrono::duration<double> delta = std::chrono::seconds(0);
	while (true)
	{
		delta += std::chrono::system_clock::now() - last;
		last = std::chrono::system_clock::now();

		// print information periodically
		if (delta.count() >= 5.0)
		{
			for (auto& wrappedSocket : m_sockets)
			{
				size_t bytesSent = wrappedSocket.get()->getSentBytes();
				std::printf("Mbytes/sec: %f, Gbytes/sec: %f, Mbits/sec: %f, Gbits/sec: %f\n",
					bytesSent / 1.0e6 / delta.count(),
					bytesSent / 1.0e9 / delta.count(),
					8 * bytesSent / 1.0e6 / delta.count(),
					8 * bytesSent / 1.0e9 / delta.count());
				wrappedSocket.get()->resetSentBytes();
				
			}
			// reset accumulators
			delta = std::chrono::seconds(0);
		}
	}
}

// a loop to keep the asio context running as long as the client/server is alive.
void BoostPerfClient::runIoContext() 
{
	while (m_runClient)
	{
		m_ioc.run();
	}
}

// called under lock
// go over alls sockets in the vector and dispatch it for the job desired.
void BoostPerfClient::dispatchSockets() 
{
	for (auto& wrappedSocket : m_sockets)
	{
		if (!wrappedSocket.get()->m_dispatched)
		{
			std::cout << "remote endpoint: " << wrappedSocket.get()->m_socket.remote_endpoint() << '\n';
			std::cout << "Am I connected? " << wrappedSocket.get()->m_connected << '\n';
			std::cout << "Am I dispatched? " << wrappedSocket.get()->m_dispatched << '\n';
			wrappedSocket.get()->m_dispatched = true;
			wrappedSocket.get()->dispatchSocketForWrite();
			//wrappedSocket.dispatchSocketForListen();
		}
	}
}

void BoostPerfClient::cleanupClosedSockets()
{
	m_sockets.erase(std::remove_if(m_sockets.begin(), m_sockets.end(), [](const std::shared_ptr<WrappedSocket>& ws)-> bool
		{
			std::cout << "on erase lamb\n";
			return ((ws.get()->m_connected == false) && (ws.get()->m_dispatched == true));
		}), m_sockets.end());
}

