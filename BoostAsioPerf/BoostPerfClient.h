#ifndef BOOST_PERF_CLIENT
#define BOOST_PERF_CLIENT
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <mutex>
#include <memory>
#include <iostream>
#include <vector>
#include "IBoostPerf.h"
#include "WrappedSocket.h"
#include "BytesConversion.h"

template <typename SocketType>
class BoostPerfClient : public IBoostPerf<SocketType>
{
public:
	BoostPerfClient(boost::asio::io_context& ioc,std::string hostname, int port,size_t socketsToReserve);
	
	void addSocket() override;
	void cleanupClosedSockets() override;
	void initializeTempSocket() override;
	void printBytes();
	void runPeriodicPrint();
	void runPeriodicCleanup();
private:
	boost::asio::io_context& m_ioc;
	std::mutex m_socketsMtx{};
	size_t m_numOfSockets;
	std::vector<std::shared_ptr<SocketType>> m_sockets;
	std::shared_ptr<SocketType> m_tempSocket;
	boost::asio::ip::tcp::resolver m_resolver;
	boost::asio::ip::tcp::resolver::results_type m_endpoint;
	boost::asio::deadline_timer m_printTimer;
	boost::asio::deadline_timer m_cleanupTimer;
};



template<typename SocketType>
inline BoostPerfClient<SocketType>::BoostPerfClient(boost::asio::io_context& ioc, std::string hostname, int port, size_t socketsToReserve) :
	m_ioc{ ioc }, m_resolver{ m_ioc }, m_endpoint{ m_resolver.resolve(std::move(hostname),std::to_string(port)) }
	, m_tempSocket{ std::make_shared<SocketType>(m_ioc) }, m_printTimer{ m_ioc }, m_cleanupTimer{m_ioc},m_numOfSockets(socketsToReserve)
{
	m_sockets.reserve(m_numOfSockets);
	initializeTempSocket();
	runPeriodicPrint();
	runPeriodicCleanup();
}

// connects a temporary socket to the endpoint
template<typename SocketType>
inline void BoostPerfClient<SocketType>::initializeTempSocket()
{
	boost::asio::async_connect(m_tempSocket.get()->m_socket, m_endpoint, 
		[this](boost::system::error_code ec, const boost::asio::ip::tcp::endpoint& endpoint)
		{
			if (!ec)
			{
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
template<typename SocketType>
inline void BoostPerfClient<SocketType>::addSocket()
{
	if (m_sockets.size() < m_numOfSockets)
	{
		std::cout << "starting AddSocket()\n";
		std::lock_guard<std::mutex> lg(m_socketsMtx);
		m_sockets.emplace_back(m_tempSocket);
		m_tempSocket = std::make_shared<SocketType>(m_ioc); // reset the temp socket after it was moved to the 
		m_sockets[m_sockets.size() - 1].get()->dispatchSocketForWrite(); // dispatch last socket added, we are under a lock so it's safe to assume it's valid.
		initializeTempSocket(); // calls async connect on the temp socket if more sockets are required.
	}
}


//clean up sockets which are disconnected and was dispatched.
template<typename SocketType>
inline void BoostPerfClient<SocketType>::cleanupClosedSockets()
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
inline void BoostPerfClient<SocketType>::printBytes()
{

	for (size_t i = 0; i < m_sockets.size(); i++)
	{
		size_t bytesProcessedBySocket = m_sockets[i].get()->m_bytesProcessed;
		m_sockets[i].get()->m_bytesProcessed = 0; // reset for next iteration
		//std::cout << bytesSentBySocket << '\n';
		std::cout << "[" << i << "]=" << bytesToMbytes(bytesProcessedBySocket) / 5  << "Mbytes/s ";
	}
	std::cout << "\n- - - - - - - - - - - - - - - - - - - - - -\n";
	runPeriodicPrint();
}


template<typename SocketType>
inline void BoostPerfClient<SocketType>::runPeriodicPrint()
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
		}); // could use std::bind 
}

template<typename SocketType>
inline void BoostPerfClient<SocketType>::runPeriodicCleanup()
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





#endif //BOOST_PERF_CLIENT

//template<typename SocketType>
//inline void BoostPerfClient<SocketType>::runSocketsLoop()
//{
//	throw std::exception("not implemented\n");
//	{
//		std::unique_lock<std::mutex> lk(m_socketsMtx);
//		std::cout << "About to wait on CV\n";
//		// need to fix the race condition here, on our last sockets, if we call addSockets() and only then m_dispatchCv.wait()'s thread is scheduled, we will wait for ever without dispatching these sockets.
//		//
//		//m_dispatchCv.wait(lk); // woke up from addSocket
//		
//		std::cout << "calling dispatch\n";
//		dispatchSocketsForWork();
//		cleanupClosedSockets();
//	}
//}

////starts loop on sockets which are not started yet.
//template<typename SocketType>
//inline void BoostPerfClient<SocketType>::dispatchSocketsForWork()
//{
//	for (auto& wrappedSocket : m_sockets)
//	{
//		if (!wrappedSocket.get()->m_dispatched)
//		{
//			wrappedSocket.get()->dispatchSocketForWrite();
//		}
//	}
//}
//called from runPeriodicPrint();
//template<typename SocketType>
//inline void BoostPerfClient<SocketType>::printSentBytes()
//{
//	for (auto& wrappedSocket : m_sockets)
//	{
//		size_t bytesSent = wrappedSocket.get()->m_sentBytes;
//		std::cout << bytesSent << '\n';
//		std::printf("Mbytes/sec: %f, Gbytes/sec: %f, Mbits/sec: %f, Gbits/sec: %f\n",
//			bytesSent / 1.0e6 / 5,
//			bytesSent / 1.0e9 / 5,
//			8 * bytesSent / 1.0e6 / 5,
//			8 * bytesSent / 1.0e9 / 5);
//		wrappedSocket.get()->m_sentBytes = 0;
//	}
//	runPeriodicPrint();
//}