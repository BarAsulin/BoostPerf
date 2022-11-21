#ifndef BOOST_PERF_SERVER
#define BOOST_PERF_SERVER
#include "BoostPerfImpl.h"
#include "BytesConversion.h"
#include "WrappedSocket.h"
#include <boost/asio.hpp>
#include <memory>
#include <iostream>

namespace ba = boost::asio;

template <typename SocketType>
class BoostPerfServer : public BoostPerfImpl<SocketType>
{
public:
	BoostPerfServer(ba::io_context& ioc, int port, size_t socketsToReserve);
	virtual	void addSocket() override;
	virtual void initializeTempSocket() override;

private:
	ba::ip::tcp::acceptor m_acceptor;
};

template<typename SocketType>
inline BoostPerfServer<SocketType>::BoostPerfServer(ba::io_context& ioc, int port, size_t socketsToReserve) : BoostPerfImpl<SocketType>(ioc, socketsToReserve), m_acceptor{ ioc,ba::ip::tcp::endpoint(ba::ip::tcp::v4(),port) }
{
	initializeTempSocket();
	BoostPerfImpl<SocketType>::runPeriodicPrint();
	BoostPerfImpl<SocketType>::runPeriodicCleanup();
}

template<typename SocketType>
inline void BoostPerfServer<SocketType>::addSocket()
{
	std::unique_lock<std::shared_mutex> lock(this->m_socketsMtx);
	this->m_sockets.emplace_back(this->m_tempSocket);
	this->m_tempSocket = std::make_shared<SocketType>(this->m_ioc); // reset the temp socket after it was moved to the 
	this->m_sockets[this->m_sockets.size() - 1].get()->dispatchSocketForRead(); // dispatch last socket added, we are under a lock so it's safe to assume it's valid.
	initializeTempSocket(); // calls async accept on the temp socket so if new connections will arrive.
}

template<typename SocketType>
inline void BoostPerfServer<SocketType>::initializeTempSocket()
{
	m_acceptor.async_accept(this->m_tempSocket.get()->m_socket, [this](boost::system::error_code ec)
	{
		if (!ec)
		{
			this->m_tempSocket.get()->m_connected = true;
			addSocket();
		}
		else
		{
			this->m_tempSocket.get()->m_socket.cancel();
			this->m_tempSocket.get()->m_socket.close();
			this->m_tempSocket.get()->m_socket = ba::ip::tcp::socket(this->m_ioc); // resets the socket
			std::cout << "Failed to establish connection with: " << this->m_tempSocket.get()->m_socket.remote_endpoint().address() << '\n';
		}
	});
}
#endif // !BOOST_PERF_SERVER