#ifndef BOOST_PERF_CLIENT
#define BOOST_PERF_CLIENT
#include "BoostPerfImpl.h"
#include <boost/asio.hpp>
#include "WrappedSocket.h"
#include <iostream>

namespace ba = boost::asio;

template <typename SocketType>
class BoostPerfClient : public BoostPerfImpl<SocketType>
{
public:
	BoostPerfClient(ba::io_context& ioc, std::string hostname,int port, size_t socketsToReserve);
	virtual	void addSocket() override;
	virtual void initializeTempSocket() override;

private:
	ba::ip::tcp::resolver m_resolver;
	ba::ip::tcp::resolver::results_type m_endpoint;
	size_t m_numOfSockets;
};

template<typename SocketType>
inline BoostPerfClient<SocketType>::BoostPerfClient(ba::io_context& ioc, std::string hostname, int port, size_t socketsToReserve) : BoostPerfImpl<SocketType>(ioc, socketsToReserve),
m_resolver{ ioc }, m_endpoint{ m_resolver.resolve(std::move(hostname),std::to_string(port)) }, m_numOfSockets{ socketsToReserve }
{
	initializeTempSocket();
	BoostPerfImpl<SocketType>::runPeriodicPrint();
	BoostPerfImpl<SocketType>::runPeriodicCleanup();
}

template<typename SocketType>
inline void BoostPerfClient<SocketType>::addSocket()
{
	std::unique_lock<std::shared_mutex> lock(this->m_socketsMtx);
	this->m_sockets.emplace_back(this->m_tempSocket);
	this->m_tempSocket = std::make_shared<SocketType>(this->m_ioc); // reset the temp socket after it was moved to the 
	this->m_sockets[this->m_sockets.size() - 1].get()->dispatchSocketForWrite(); // dispatch last socket added, we are under a lock so it's safe to assume it's valid.
	bool notEnoughSockets = (this->m_sockets.size() < m_numOfSockets);
	if (notEnoughSockets)
	{
		initializeTempSocket(); // calls async connect on the temp socket if more sockets are required
	}
}

template<typename SocketType>
inline void BoostPerfClient<SocketType>::initializeTempSocket()
{
	ba::async_connect(this->m_tempSocket.get()->m_socket, m_endpoint,
		[this](boost::system::error_code ec, const ba::ip::tcp::endpoint& endpoint)
		{
			if (!ec)
			{
				this->m_tempSocket.get()->m_connected = true;
				addSocket(); // call to move the temp socket to the sockets vector 
			}
			else
			{
				this->m_tempSocket.get()->m_socket.cancel();
				this->m_tempSocket.get()->m_socket.close();
				this->m_tempSocket.get()->m_socket = ba::ip::tcp::socket(this->m_ioc); // resets the socket
				std::cout << "Failed to connect to: " << m_endpoint.begin()->endpoint() << '\n';
			}
		});
}

#endif // !BOOST_PERF_CLIENT