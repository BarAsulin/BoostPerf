#pragma once

#include <boost/asio.hpp>
#include "WrappedMessage.h"

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
	void doReadHeader();
	void doReadBody();

	void doEncodeBeforeWrite();
	void doWriteMsg();

	void addSentBytes(size_t bytes);
	size_t getSentBytes() const;


private:
	boost::asio::io_context* m_ioc;
	WrappedMessage m_msg{};
	size_t m_sentBytes{};
};
