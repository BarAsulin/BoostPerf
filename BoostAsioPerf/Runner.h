#ifndef RUNNER_H
#define RUNNER_H
#include <boost/asio.hpp>
#include "BoostPerfClient.h"
#include "BoostPerfServer.h"
#include "BoostPerfSslClient.h"
#include "BoostPerfSslServer.h"
#include "ParsedArgv.h"
#include <iostream>
#include <boost/asio/ssl.hpp>
#include "root_certificate.h"
#include "server_certificate.h"
namespace ba = boost::asio;

struct Runner 
{
	inline static void run(const ParsedArgv& args)
	{
		if (args.m_IsClient)
		{
			if (args.m_usingSSL)
			{
				runTLSClient(args);
			}
			else
			{
				runClient(args);
			}
		}
		else if (args.m_IsServer)
		{
			if (args.m_usingSSL)
			{
				runTLSServer(args);
			}
			else
			{
				runServer(args);
			}
		}
		else
		{
			ParsedArgv::printUsage();
		}
	}
	inline static void runClient(const ParsedArgv& args)
	{
		ba::io_context ioc;
		ba::io_context::work work(ioc);
		BoostPerfClient<WrappedSocket> perfClient(ioc, args.m_serverHostname, args.m_port, args.m_sockets);
		try
		{
			ioc.run();
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	inline static void runTLSClient(const ParsedArgv& args)
	{
		ba::io_context ioc;
		ba::ssl::context ctx(boost::asio::ssl::context::tlsv12);
		load_root_certificates(ctx);
		ba::io_context::work work(ioc);
		BoostPerfSslClient<WrappedSslSocket> perfClient(ioc, ctx, args.m_serverHostname, args.m_port, args.m_sockets);
		try
		{
			ioc.run();
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	inline static void runServer(const ParsedArgv& args)
	{
		ba::io_context ioc;
		ba::io_context::work work(ioc);
		BoostPerfServer<WrappedSocket> perfServer(ioc, args.m_port, args.m_sockets);
		try
		{
			ioc.run();
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	inline static void runTLSServer(const ParsedArgv& args)
	{
		ba::io_context ioc;
		ba::ssl::context ctx(boost::asio::ssl::context::tlsv12);
		load_server_certificate(ctx);
		ba::io_context::work work(ioc);
		BoostPerfSslServer<WrappedSslSocket> perfServer(ioc, ctx, args.m_port, args.m_sockets);
		try
		{
			ioc.run();
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
};

#endif // RUNNER_H