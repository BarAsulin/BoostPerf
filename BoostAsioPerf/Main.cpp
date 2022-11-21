#include <boost/asio.hpp>
#include "BoostPerfClient.h"
#include "BoostPerfServer.h"
#include "ParsedArgv.h"
#include <iostream>


namespace ba = boost::asio;

void runClient(const ParsedArgv& args)
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

void runServer(const ParsedArgv& args)
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


int main(int argc, char** argv)
{
	if (argc < 3)
	{
		ParsedArgv::printUsage();
	}
	ParsedArgv args{ argc,argv };
	if (args.m_IsClient)
	{
		runClient(args);
	}
	else if (args.m_IsServer)
	{
		runServer(args);
	}
	else
	{
		ParsedArgv::printUsage();
	}
	return 0;
}

