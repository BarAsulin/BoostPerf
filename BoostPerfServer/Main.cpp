#include "BoostPerfServer.h"
#include "WrappedMessage.h"
#include <iostream>


int main(int argc, char* argv[])
{
	BoostPerfServer perfServer(2005, 8);
	std::thread iocThread(&BoostPerfServer::runIoContext, std::ref(perfServer));
	std::thread dispatchSocketsThread(&BoostPerfServer::runSocketsLoop, std::ref(perfServer));

	iocThread.join();
	dispatchSocketsThread.join();
	std::cin.get();

}