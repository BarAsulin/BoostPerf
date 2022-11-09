#include "BoostPerfClient.h"
#include "ParsedArgv.h"
#include <iostream>


int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cout << "not enough arguments\n";
		std::cin.get();
	}
	ParsedArgv arguments(argc, argv);
	
	BoostPerfClient	perfClient(arguments.m_serverHostname, arguments.m_port, arguments.m_sockets);
	std::thread iocThread(&BoostPerfClient::runIoContext, std::ref(perfClient));
	std::thread dispatchSocketsThread(&BoostPerfClient::runSocketsLoop, std::ref(perfClient));
	std::thread printStats(&BoostPerfClient::printSocketsStats, std::ref(perfClient));

	printStats.join();
	iocThread.join();
	dispatchSocketsThread.join();
	std::cin.get();
}