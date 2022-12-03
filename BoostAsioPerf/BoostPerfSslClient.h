#ifndef BOOST_PERF_SSL_CLIENT
#define BOOST_PERF_SSL_CLIENT
#include "BoostPerfImpl.h"
#include "WrappedSslSocket.h"
#include <iostream>


namespace ba = boost::asio;

template <typename SocketType>
class BoostPerfSslClient : public BoostPerfImpl<SocketType>
{
public:
	BoostPerfSslClient(ba::io_context& ioc, ba::ssl::context& sslCtx, std::string hostname, int port, size_t socketsToReserve);
	virtual	void addSocket() override;
	virtual void initializeTempSocket() override;
	virtual void runAsyncHandshake();
private:
	ba::ssl::context& m_sslCtx;
	ba::ip::tcp::resolver m_resolver;
	ba::ip::tcp::resolver::results_type m_endpoint;
	size_t m_numOfSockets;
};


template<typename SocketType>
inline BoostPerfSslClient<SocketType>::BoostPerfSslClient(ba::io_context& ioc, ba::ssl::context& sslCtx, std::string hostname, int port, size_t socketsToReserve) :
	BoostPerfImpl<SocketType>(ioc,sslCtx, socketsToReserve), m_sslCtx{ sslCtx }, m_resolver{ ioc },
	m_endpoint{ m_resolver.resolve(std::move(hostname),std::to_string(port)) }, m_numOfSockets{ socketsToReserve }
{
	initializeTempSocket();
	BoostPerfImpl<SocketType>::runPeriodicPrint();
	BoostPerfImpl<SocketType>::runPeriodicCleanup();
}


template<typename SocketType>
inline void BoostPerfSslClient<SocketType>::addSocket()
{
	std::unique_lock<std::shared_mutex> lock(this->m_socketsMtx);
	this->m_sockets.emplace_back(this->m_tempSocket);
	this->m_tempSocket = std::make_shared<SocketType>(this->m_ioc,this->m_sslCtx); // reset the temp socket after it was moved to the 
	this->m_sockets[this->m_sockets.size() - 1].get()->dispatchSocketForWrite(); // dispatch last socket added, we are under a lock so it's safe to assume it's valid.
	bool notEnoughSockets = (this->m_sockets.size() < m_numOfSockets);
	if (notEnoughSockets)
	{
		initializeTempSocket(); // calls async connect on the temp socket if more sockets are required
	}
}



template<typename SocketType>
inline void BoostPerfSslClient<SocketType>::initializeTempSocket()
{
	ba::async_connect(this->m_tempSocket.get()->m_socket.next_layer(), m_endpoint,
		[this](boost::system::error_code ec, const ba::ip::tcp::endpoint& endpoint)
		{
			if (!ec)
			{
				runAsyncHandshake();
			}
			else
			{
				this->m_tempSocket.get()->m_socket.next_layer().cancel();
				this->m_tempSocket.get()->m_socket.next_layer().close();
				this->m_tempSocket.get()->m_socket = sslSocket(this->m_ioc, this->m_sslCtx); // resets the socket
				std::cerr << "Failed to connect to: " << m_endpoint.begin()->endpoint() << '\n';
			}
		});
}

template<typename SocketType>
inline void BoostPerfSslClient<SocketType>::runAsyncHandshake()
{
	this->m_tempSocket.get()->m_socket.async_handshake(ba::ssl::stream_base::client,
		[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				this->m_tempSocket.get()->m_connected = true;
				addSocket(); // call to move the temp socket to the sockets vector 
			}
			else
			{
				std::cerr << "TLS Handshake failed: " << ec.message() << "\n";
			}
		});

}







#endif // BOOST_PERF_SSL_CLIENT
