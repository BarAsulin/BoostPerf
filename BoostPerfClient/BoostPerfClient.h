#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "WrappedSocket.h"

class BoostPerfClient
{
public:
	BoostPerfClient(const std::string& hostname, int port, size_t numOfStreams);
    void runIoContext();
    void runSocketsLoop();
private:
    void addSocket();
    void dispatchSockets();
    void cleanupClosedSockets();
    void doConnect();
private:
    boost::asio::io_context m_ioc{};
    int m_port;
    size_t m_numOfStreams;
    std::mutex m_socketsMtx{};
    std::condition_variable m_dispatch_cv{};
    std::vector<std::shared_ptr<WrappedSocket>> m_sockets{};
    std::shared_ptr<WrappedSocket> m_tempSocket;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ip::tcp::resolver::results_type m_endpoint;
    std::atomic_bool m_runClient = false;

};

