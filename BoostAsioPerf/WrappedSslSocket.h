#ifndef WRAPPED_SSL_SOCKET_H
#define WRAPPED_SSL_SOCKET_H
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>


using sslSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
class WrappedSslSocket
{
public:
	WrappedSslSocket(boost::asio::io_context& ioc, boost::asio::ssl::context& ssl_ctx);
	~WrappedSslSocket();
	void dispatchSocketForRead();
	void dispatchSocketForWrite();
	void doWrite();
	void doRead();

	std::atomic<size_t> m_bytesProcessed{};
	std::atomic<bool> m_dispatched{ false };
	std::atomic<bool> m_connected{ false };
	sslSocket m_socket;
private:
	std::vector<unsigned char> m_buffer;
};

#endif // !WRAPPED_SSL_SOCKET_H