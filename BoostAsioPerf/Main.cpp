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
	if (args.m_IsServer)
	{
		runServer(args);
	}
	return 0;
}

//TODO test use cases when the socket is forcibly closed by either party of the connection
//TODO ensure only m_numOfSockets are added to the vector.
