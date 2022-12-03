#include "WrappedSslSocket.h"
#include <iostream>


namespace ba = boost::asio; 

WrappedSslSocket::WrappedSslSocket(boost::asio::io_context& ioc, boost::asio::ssl::context& ssl_ctx) : m_socket(ioc,ssl_ctx), m_buffer(131072, 0) //128KiB
{

}

WrappedSslSocket::~WrappedSslSocket()
{
	m_socket.next_layer().close();
}

void WrappedSslSocket::dispatchSocketForRead()
{
	m_dispatched = true;
	doRead();
}

void WrappedSslSocket::dispatchSocketForWrite()
{
    m_dispatched = true;
    doWrite();
}

void WrappedSslSocket::doWrite()
{
    ba::async_write(m_socket, ba::buffer(m_buffer),
        [this](boost::system::error_code ec, size_t length)
        {
            if (!ec)
            {
                m_bytesProcessed += length;
                doWrite();
            }
            else
            {
                std::cerr << "Error on socket: " << ec.message() << '\n';
                m_socket.next_layer().cancel();
                m_socket.next_layer().close();
                m_connected = false;
            }
        });
}

void WrappedSslSocket::doRead()
{
    
    m_socket.next_layer().async_receive(ba::buffer(m_buffer), 0,
        [this](auto ec, auto bytes) {
            if (!ec)
            {
                m_bytesProcessed += bytes;
                doRead();
            }
            else
            {
                std::cerr << "Error on socket: " << ec.message() << '\n';
                m_socket.next_layer().cancel();
                m_socket.next_layer().close();
                m_connected = false;
            }
        });
}
