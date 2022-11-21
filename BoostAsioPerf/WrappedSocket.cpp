#include "WrappedSocket.h"
#include <iostream>

namespace ba = boost::asio;
WrappedSocket::WrappedSocket(ba::io_context& ioc) : m_socket(ioc), m_buffer(131072,0) //128KiB
{}

WrappedSocket::~WrappedSocket()
{
	m_socket.close();
}

void WrappedSocket::dispatchSocketForRead()
{
    m_dispatched = true;
	doRead();
}

void WrappedSocket::doRead()
{
    m_socket.async_receive(ba::buffer(m_buffer), 0,
        [this](auto ec, auto bytes) {
            if (!ec)
            {
                m_bytesProcessed += bytes;
                doRead();
            }
            else
            {
                std::cerr << "Error on socket: " << ec.message() << '\n';
                m_socket.cancel();
                m_socket.close();
                m_connected = false;
            }
        });
}

void WrappedSocket::dispatchSocketForWrite()
{
    m_dispatched = true;
	doWrite();
}

void WrappedSocket::doWrite()
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
                m_socket.cancel();
                m_socket.close();
                m_connected = false;
            }
        });
}

