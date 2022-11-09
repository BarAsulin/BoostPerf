#include "BoostPerfServer.h"
#include <iostream>
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

BoostPerfServer::BoostPerfServer(int port, size_t numOfStreams) : m_port{ port }, m_numOfStreams{ numOfStreams }, m_resolver{ make_strand(m_ioc) },
m_acceptor{ make_strand(m_ioc),tcp::endpoint(tcp::v4(), m_port) }, m_tempSocket{ std::make_shared<WrappedSocket>(m_ioc) }
{
    m_sockets.reserve(m_numOfStreams);
    m_runServer = true;
    doAccept();
}
void BoostPerfServer::doAccept()
{
    m_acceptor.async_accept(m_tempSocket.get()->m_socket, [this](boost::system::error_code ec)
        {
            if (!ec)
            {
                m_tempSocket.get()->m_connected = true;
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
        m_sockets.emplace_back(m_tempSocket);
        m_tempSocket = std::make_shared<WrappedSocket>(m_ioc); // reset the temp socket
        this->doAccept();
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
    m_sockets.erase(std::remove_if(m_sockets.begin(), m_sockets.end(), [](const std::shared_ptr<WrappedSocket>& ws)-> bool
        {
            std::cout << "on erase lamb\n";
            return ((ws.get()->m_connected == false) && (ws.get()->m_dispatched == true));
        }), m_sockets.end());
}
void BoostPerfServer::dispatchSockets()
{
    for (auto& wrappedSocket : m_sockets)
    {
        if (!wrappedSocket.get()->m_dispatched)
        {
            std::cout << "remote endpoint: " << wrappedSocket.get()->m_socket.remote_endpoint() << '\n';
            std::cout << "Am I connected? " << wrappedSocket.get()->m_connected << '\n';
            std::cout << "Am I dispatched? " << wrappedSocket.get()->m_dispatched << '\n';
            wrappedSocket.get()->m_dispatched = true;
            wrappedSocket.get()->dispatchSocketForListen();
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
