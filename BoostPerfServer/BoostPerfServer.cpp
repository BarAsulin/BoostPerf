#include "BoostPerfServer.h"
#include <iostream>
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

BoostPerfServer::BoostPerfServer(int port, size_t numOfStreams) : m_port{ port }, m_numOfStreams{ numOfStreams }, m_resolver{ make_strand(m_ioc) },
m_acceptor{ make_strand(m_ioc),tcp::endpoint(tcp::v4(), m_port) }, m_tempSocket{ m_ioc }
{
    m_sockets.reserve(m_numOfStreams);
    m_runServer = true;
    do_accept();
}
void BoostPerfServer::do_accept()
{
    m_acceptor.async_accept(m_tempSocket.m_socket, [this](boost::system::error_code ec)
        {
            if (!ec)
            {
                m_tempSocket.m_connected = true;
                std::cout << "calling addsocket\n";
                addSocket();
            }
        });

}
void BoostPerfServer::addSocket()
{
    if (m_sockets.size() < m_numOfStreams)
    {
        std::lock_guard<std::mutex> lg(m_socketsMtx);
        m_sockets.emplace_back(std::move(m_tempSocket));
        this->do_accept();
        m_dispatch_cv.notify_one();
    }
}
void BoostPerfServer::runSocketsLoop()
{
    while (true)
    {
        std::unique_lock<std::mutex> lk(m_socketsMtx);
        m_dispatch_cv.wait(lk);
        std::cout << "starting for, size of vec is: " << m_sockets.size() << '\n';
        dispatchSockets();
        cleanupClosedSockets();
    }
}
void BoostPerfServer::cleanupClosedSockets()
{
    m_sockets.erase(std::remove_if(m_sockets.begin(), m_sockets.end(), [](const WrappedSocket& ws)-> bool
        {
            std::cout << "on erase lamb\n";
            return ((ws.m_connected == false) && (ws.m_dispatched == true));
        }), m_sockets.end());
}
void BoostPerfServer::dispatchSockets()
{
    for (auto& wrappedSocket : m_sockets)
    {
        if (!wrappedSocket.m_dispatched)
        {
            std::cout << "remote endpoint: " << wrappedSocket.m_socket.remote_endpoint() << '\n';
            std::cout << "Am I connected? " << wrappedSocket.m_connected << '\n';
            std::cout << "Am I dispatched? " << wrappedSocket.m_dispatched << '\n';
            wrappedSocket.m_dispatched = true;
            wrappedSocket.dispatchSocketForListen();
        }
    }
}
void BoostPerfServer::runIoContext()
{
    while (m_runServer)
    {
        m_ioc.run();
    }
}
