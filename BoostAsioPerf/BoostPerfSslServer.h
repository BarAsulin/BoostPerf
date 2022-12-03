#ifndef BOOST_PERF_SSL_SERVER
#define BOOST_PERF_SSL_SERVER

#include "BoostPerfImpl.h"
#include "WrappedSslSocket.h"
#include <boost/asio.hpp>
#include <iostream>


namespace ba = boost::asio;

template <typename SocketType>
class BoostPerfSslServer : public BoostPerfImpl<SocketType>
{
public:
	BoostPerfSslServer(ba::io_context& ioc, ba::ssl::context& sslCtx, int port, size_t socketsToReserve);
	virtual	void addSocket() override;
	virtual void initializeTempSocket() override;
	virtual void runAsyncHandshake();
private:
	ba::ssl::context& m_sslCtx;
	ba::ip::tcp::acceptor m_acceptor;
};




template<typename SocketType>
inline BoostPerfSslServer<SocketType>::BoostPerfSslServer(ba::io_context& ioc, ba::ssl::context& sslCtx, int port, size_t socketsToReserve) : 
	BoostPerfImpl<SocketType>(ioc, sslCtx, socketsToReserve), m_sslCtx{sslCtx}, m_acceptor{ ioc,ba::ip::tcp::endpoint(ba::ip::tcp::v4(),port) }
{
	initializeTempSocket();
	BoostPerfImpl<SocketType>::runPeriodicPrint();
	BoostPerfImpl<SocketType>::runPeriodicCleanup();
}
template<typename SocketType>
inline void BoostPerfSslServer<SocketType>::addSocket()
{
	std::unique_lock<std::shared_mutex> lock(this->m_socketsMtx);
	this->m_sockets.emplace_back(this->m_tempSocket);
	this->m_tempSocket = std::make_shared<SocketType>(this->m_ioc,this->m_sslCtx); // reset the temp socket after it was moved to the 
	this->m_sockets[this->m_sockets.size() - 1].get()->dispatchSocketForRead(); // dispatch last socket added, we are under a lock so it's safe to assume it's valid.
	initializeTempSocket(); // calls async accept on the temp socket so if new connections will arrive.
}
template<typename SocketType>
inline void BoostPerfSslServer<SocketType>::initializeTempSocket()
{
	m_acceptor.async_accept(this->m_tempSocket.get()->m_socket.next_layer(), [this](boost::system::error_code ec)
		{
			if (!ec)
			{
				runAsyncHandshake();
			}
			else
			{
				this->m_tempSocket.get()->m_socket.next_layer().cancel();
				this->m_tempSocket.get()->m_socket.next_layer().close();
				this->m_tempSocket.get()->m_socket = sslSocket(this->m_ioc,this->m_sslCtx); // resets the socket
				std::cout << "Failed to establish connection with: " << this->m_tempSocket.get()->m_socket.next_layer().remote_endpoint().address() << '\n';
			}
		});
}
template<typename SocketType>
inline void BoostPerfSslServer<SocketType>::runAsyncHandshake()
{
	this->m_tempSocket.get()->m_socket.async_handshake(ba::ssl::stream_base::server,
		[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				this->m_tempSocket.get()->m_connected = true;
				addSocket();
			}
			else
			{
				std::cerr << "TLS Handshake failed: " << ec.message() << "\n";
			}
		});
}
#endif //BOOST_PERF_SSL_SERVER