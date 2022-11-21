#ifndef BOOST_PERF_IMPL
#define BOOST_PERF_IMPL
#include "IBoostPerf.h"
#include "WrappedSocket.h"
#include "BytesConversion.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <shared_mutex>
#include <memory>
#include <iostream>

namespace ba = boost::asio;

template <typename SocketType>
class BoostPerfImpl : public IBoostPerf<SocketType>
{
public:
	BoostPerfImpl(ba::io_context& ioc, size_t socketsToReserve);
	virtual ~BoostPerfImpl();
	virtual void addSocket() = 0;
	virtual void initializeTempSocket() = 0;
	virtual void cleanupClosedSockets() override;
	virtual void printBytes() override;
	virtual void runPeriodicPrint() override;
	virtual void runPeriodicCleanup() override;

protected:
	ba::io_context& m_ioc;
	std::vector<std::shared_ptr<SocketType>> m_sockets;
	std::shared_ptr<SocketType> m_tempSocket;
	std::shared_mutex m_socketsMtx{};
	ba::deadline_timer m_printTimer;
	ba::deadline_timer m_cleanupTimer;
};

template<typename SocketType>
inline BoostPerfImpl<SocketType>::~BoostPerfImpl() {}


template<typename SocketType>
inline BoostPerfImpl<SocketType>::BoostPerfImpl(ba::io_context& ioc, size_t socketsToReserve) : m_ioc{ ioc }, m_tempSocket{ std::make_shared<SocketType>(m_ioc) }, m_printTimer{ ioc }, m_cleanupTimer{ ioc }
{
	m_sockets.reserve(socketsToReserve);
}

template<typename SocketType>
inline void BoostPerfImpl<SocketType>::cleanupClosedSockets()
{
	m_sockets.erase(std::remove_if(m_sockets.begin(), m_sockets.end(), [](const std::shared_ptr<SocketType>& ws)-> bool
		{
			bool dispatchedAndDisconnected = ((ws.get()->m_connected == false) && (ws.get()->m_dispatched == true));
			if (dispatchedAndDisconnected)
			{
				std::cout << "About to remove socket as it's disconnected\n";
			}
			return dispatchedAndDisconnected;
		}), m_sockets.end());
	runPeriodicCleanup();
}

template<typename SocketType>
inline void BoostPerfImpl<SocketType>::printBytes()
{
	size_t sumOfAllSocketsInMbytes{};
	std::shared_lock<std::shared_mutex> lock(m_socketsMtx);
	for (size_t i = 0; i < m_sockets.size(); i++)
	{
		size_t bytesProcessedBySocket = m_sockets[i].get()->m_bytesProcessed;
		m_sockets[i].get()->m_bytesProcessed = 0; // reset for next iteration
		size_t MbytesProcessedBySocket = bytesToMbytes(bytesProcessedBySocket);
		sumOfAllSocketsInMbytes += MbytesProcessedBySocket;
		std::cout << "[" << i << "]=" << MbytesProcessedBySocket / 5 << "Mbytes/s ";
	}
	std::cout << "\n- - - Avg socket in Mbytes/s:" << sumOfAllSocketsInMbytes / m_sockets.size() / 5 << " - - - Total for sockets in Mbytes/s:" << sumOfAllSocketsInMbytes / 5 << " - - -\n";
	runPeriodicPrint();
}

template<typename SocketType>
inline void BoostPerfImpl<SocketType>::runPeriodicPrint()
{
	m_printTimer.expires_from_now(boost::posix_time::seconds(5));
	m_printTimer.async_wait([this](boost::system::error_code ec)
		{
			if (!ec)
			{
				{
					std::shared_lock<std::shared_mutex> lock(m_socketsMtx); // ensure the timer doesn't run before there are no sockets.
					if (m_sockets.size() == 0)
					{
						runPeriodicPrint();
						return;
					}
				}
				printBytes();
			}
			else
			{
				std::cerr << "Error on printing data: ";
				std::cerr << ec.message() << '\n';
				return;
			}
		}); // could use std::bind 
}

template<typename SocketType>
inline void BoostPerfImpl<SocketType>::runPeriodicCleanup()
{
	m_cleanupTimer.expires_from_now(boost::posix_time::seconds(10));
	m_cleanupTimer.async_wait([this](boost::system::error_code ec)
		{
			if (!ec)
			{
				{
					std::shared_lock<std::shared_mutex> lock(m_socketsMtx);
					if (m_sockets.size() == 0) // ensure the timer doesn't run before there are no sockets.
					{
						runPeriodicCleanup();
						return;
					}
				}
				cleanupClosedSockets();
			}
			else
			{
				std::cerr << "Error on cleaning up closed sockets: ";
				std::cerr << ec.message() << '\n';
				return;
			}
		});
}

#endif // BOOST_PERF_IMPL
