#include <boost/asio.hpp>
#include "BoostPerfClient.h"
#include "BoostPerfServer.h"
#include "WrappedSocket.h"
#include "ParsedArgv.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
namespace ba = boost::asio;


void runClient(const ParsedArgv& args)
{
	ba::io_context ioc;
	ba::io_context::work work(ioc);
	BoostPerfClient<WrappedSocket> perfClient(ioc, args.m_serverHostname, args.m_port, args.m_sockets);
	ioc.run();
}

void runServer(const ParsedArgv& args)
{
	ba::io_context ioc;
	ba::io_context::work work(ioc);
	BoostPerfServer<WrappedSocket> perfServer(ioc, args.m_port, args.m_sockets);
	ioc.run();
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

//TODO test use cases when the socket is forcibly closed by either party of the connection -- can use windows firewall.
//TODO catch exceptions.
//TODO change m_socketsMtx to be a shared_mutex.
//TODO move implementations to BoostPerf.