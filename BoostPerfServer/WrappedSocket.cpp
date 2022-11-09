
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
    this->doReadHeader();
}


void WrappedSocket::doReadHeader()
{
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_msg.data(), WrappedMessage::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec && m_msg.decodeHeader())
            {
                this->doReadBody();
            }
            else
            {
                std::cout << "reached the else in do_read_header\n";
                m_socket.close();
                m_connected = false;
            }
        });
}

void WrappedSocket::doReadBody()
{
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_msg.body(), m_msg.getBodyLength()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                std::cout.write(m_msg.body(), m_msg.getBodyLength());
                std::cout << '\n';
                this->doReadHeader();
            }
            else
            {
                std::cout << "reached the else in do_read_body\n";
                m_socket.close();
                m_connected = false;
            }
        });
}

void WrappedSocket::doEncodeBeforeWrite()
{
    memcpy(m_msg.body(), WrappedMessage::exampleDataToSend.data(), WrappedMessage::exampleDataToSend.size()); // this is intended to simulate some work with the data sent/received.
    m_msg.setBodyLength(WrappedMessage::exampleDataToSend.size());
    m_msg.encodeHeader();
    doWriteMsg();
}

void WrappedSocket::doWriteMsg()
{
    boost::asio::async_write(m_socket,
        boost::asio::buffer(m_msg.data(), m_msg.length()),
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                doEncodeBeforeWrite();
            }
            else
            {
                m_socket.close();
                m_connected = false;
            }
        });
}


void WrappedSocket::dispatchSocketForWrite()
{
    this->doEncodeBeforeWrite();
}

