#ifndef BOOST_PERF_SERVER
#define BOOST_PERF_SERVER
#include "IBoostPerf.h"
#include "BytesConversion.h"
#include "WrappedSocket.h"
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <iostream>

namespace ba = boost::asio;

template <typename SocketType>
class BoostPerfServer : public IBoostPerf<SocketType>
{
public:
	BoostPerfServer(ba::io_context& ioc, int port,size_t numOfSockets = 0);


	void addSocket() override;
	void cleanupClosedSockets() override;
	void initializeTempSocket() override;
	void printBytes();
	void runPeriodicPrint();
	void runPeriodicCleanup();
private:
	ba::io_context& m_ioc;
	std::vector<std::shared_ptr<SocketType>> m_sockets;
	std::shared_ptr<SocketType> m_tempSocket;
	ba::ip::tcp::acceptor m_acceptor;
	std::mutex m_socketsMtx{};
	ba::deadline_timer m_printTimer;
	ba::deadline_timer m_cleanupTimer;
};

template<typename SocketType>
inline BoostPerfServer<SocketType>::BoostPerfServer(ba::io_context& ioc, int port, size_t numOfSockets) : m_ioc{ ioc }, m_tempSocket{std::make_shared<SocketType>(m_ioc)},
	m_acceptor{ m_ioc,ba::ip::tcp::endpoint(ba::ip::tcp::v4(),port)}, m_printTimer{m_ioc}, m_cleanupTimer{m_ioc}
{
	m_sockets.reserve(numOfSockets);
	initializeTempSocket();
	runPeriodicPrint();
	runPeriodicCleanup();
}

template<typename SocketType>
inline void BoostPerfServer<SocketType>::initializeTempSocket()
{
	m_acceptor.async_accept(m_tempSocket.get()->m_socket, [this](boost::system::error_code ec)
		{
			if (!ec)
			{
				m_tempSocket.get()->m_connected = true;
				addSocket();
			}
			else
			{
				m_tempSocket.get()->m_socket.cancel();
				m_tempSocket.get()->m_socket.close();
				m_tempSocket.get()->m_socket = boost::asio::ip::tcp::socket(m_ioc); // resets the socket
				std::cout << "Failed to establish connection with: " << m_tempSocket.get()->m_socket.remote_endpoint().address() << '\n';
			}
		});
}
template<typename SocketType>
inline void BoostPerfServer<SocketType>::addSocket()
{
	std::lock_guard<std::mutex> lg(m_socketsMtx);
	m_sockets.emplace_back(m_tempSocket);
	m_tempSocket = std::make_shared<SocketType>(m_ioc); // reset the temp socket after it was moved to the 
	m_sockets[m_sockets.size() - 1].get()->dispatchSocketForRead(); // dispatch last socket added, we are under a lock so it's safe to assume it's valid.
	initializeTempSocket(); // calls async accept on the temp socket so if new connections will arrive.
}

template<typename SocketType>
inline void BoostPerfServer<SocketType>::cleanupClosedSockets()
{
	m_sockets.erase(std::remove_if(m_sockets.begin(), m_sockets.end(), [](const std::shared_ptr<SocketType>& ws)-> bool
		{
			bool dispatchedAndDisconnected = ((ws.get()->m_connected == false) && (ws.get()->m_dispatched == true));
			if (dispatchedAndDisconnected)
			{
				std::cout << "About to remove socket as it's disconnected\n";
			}
			return dispatchedAndDisconnected;
		}), m_sockets.end());
	runPeriodicCleanup();
}



template<typename SocketType>
inline void BoostPerfServer<SocketType>::printBytes()
{
	for (size_t i = 0; i < m_sockets.size(); i++)
	{
		size_t bytesProcessedBySocket = m_sockets[i].get()->m_bytesProcessed;
		m_sockets[i].get()->m_bytesProcessed = 0; // reset for next iteration
		std::cout << "[" << i << "]=" << bytesToMbytes(bytesProcessedBySocket) / 5 << "Mbytes/s ";
	}
	std::cout << "\n- - - - - - - - - - - - - - - - - - - - - -\n";
	runPeriodicPrint();
}

template<typename SocketType>
inline void BoostPerfServer<SocketType>::runPeriodicPrint()
{
	m_printTimer.expires_from_now(boost::posix_time::seconds(5));
	m_printTimer.async_wait([this](boost::system::error_code ec)
		{
			if (!ec)
			{
				printBytes();
			}
			else
			{
				std::cerr << "Error on printing data:\n";
				std::cerr << ec.message() << '\n';
				return;
			}
		});
}

template<typename SocketType>
inline void BoostPerfServer<SocketType>::runPeriodicCleanup()
{
	m_cleanupTimer.expires_from_now(boost::posix_time::seconds(10));
	m_cleanupTimer.async_wait([this](boost::system::error_code ec)
		{
			if (!ec)
			{
				cleanupClosedSockets();
			}
			else
			{
				std::cerr << "Error on cleaning up closed sockets:\n";
				std::cerr << ec.message() << '\n';
				return;
			}
		});
}























#endif //BOOST_PERF_SERVER