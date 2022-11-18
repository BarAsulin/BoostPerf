#pragma once

#include <boost/asio.hpp>
#include "WrappedMessage.h"

typedef boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO> rcv_timeout_option; //somewhere in your headers to be used everywhere you need it
typedef boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_SNDTIMEO> snd_timeout_option;

class WrappedSocket
{
public:
	WrappedSocket(boost::asio::io_context& ioc);

	bool m_connected{ false };
	bool m_dispatched{ false };
	boost::asio::ip::tcp::socket m_socket;
	void dispatchSocketForListen();
	void dispatchSocketForWrite();
	void addSentBytes(size_t bytes);
	size_t getSentBytes() const;
	void resetSentBytes();
private:
	void doReadHeader();
	void doReadBody();

	void doEncodeBeforeWrite();
	void doWriteMsg();

	

private:
	boost::asio::io_context* m_ioc;
	WrappedMessage m_msg{};
	size_t m_sentBytes{};
	std::vector<unsigned char> m_buffer;
};


//void setSocketTimeout(int rcvTimeoutMiliseconds, int sndTimeoutMiliseconds);