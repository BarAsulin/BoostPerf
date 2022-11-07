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

private:
	void do_read_header();
	void do_read_body();

	void do_encode_msg();
	void do_write_msg();

	void addSentBytes(size_t bytes);
	size_t getSentBytes() const;
	

private:
	boost::asio::io_context* m_ioc;
	WrappedMessage m_msg{};
	size_t m_sentBytes{};
};


//void setSocketTimeout(int rcvTimeoutMiliseconds, int sndTimeoutMiliseconds);