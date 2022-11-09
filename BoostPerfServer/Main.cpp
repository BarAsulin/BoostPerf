#include "BoostPerfServer.h"
#include "WrappedMessage.h"
#include "ParsedArgv.h"
#include <iostream>



int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "not enough arguments\n";
		std::cin.get();
	}
	ParsedArgv arguments(argc, argv);

	BoostPerfServer perfServer(arguments.m_port, arguments.m_sockets);
	std::thread iocThread(&BoostPerfServer::runIoContext, std::ref(perfServer));
	std::thread dispatchSocketsThread(&BoostPerfServer::runSocketsLoop, std::ref(perfServer));

	iocThread.join();
	dispatchSocketsThread.join();
	std::cin.get();

}