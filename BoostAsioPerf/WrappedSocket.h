#pragma once
#include <boost/asio.hpp>
#include <atomic>
#include <vector>

class WrappedSocket
{
public:
	WrappedSocket(boost::asio::io_context& ioc);
	~WrappedSocket();
	void dispatchSocketForRead();
	void dispatchSocketForWrite();
	void doWrite();
	void doRead();

	std::atomic<size_t> m_bytesProcessed{};
	std::atomic<bool> m_dispatched{false};
	std::atomic<bool> m_connected{false};
	boost::asio::ip::tcp::socket m_socket;
private:
	std::vector<unsigned char> m_buffer;
};

