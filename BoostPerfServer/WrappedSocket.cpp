
#include <boost/asio.hpp>
#include <iostream>
#include "WrappedSocket.h"

void WrappedSocket::addSentBytes(size_t bytes)
{
	this->m_sentBytes += bytes;
}

size_t WrappedSocket::getSentBytes() const
{
	return m_sentBytes;
}


WrappedSocket::WrappedSocket(boost::asio::io_context& ioc) : m_socket(boost::asio::ip::tcp::socket(ioc))
{
    m_ioc = std::addressof(ioc);
}



void WrappedSocket::dispatchSocketForListen()
{
	this->do_read_header();
}

void WrappedSocket::do_read_header()
{
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_msg.data(), WrappedMessage::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec && m_msg.decode_header())
            {
                this->do_read_body();
            }
            else
            {
                std::cout << "reached the else in do_read_header\n";
                m_socket.close();
                m_connected = false;
            }
        });
}
void WrappedSocket::do_read_body()
{
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_msg.body(), m_msg.body_length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                std::cout.write(m_msg.body(), m_msg.body_length());
                std::cout << '\n';
                this->do_read_header();
            }
            else
            {
                std::cout << "reached the else in do_read_body\n";
                m_socket.close();
                m_connected = false;
            }
        });
}



