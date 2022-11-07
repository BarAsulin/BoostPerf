#include "BoostPerfClient.h"
#include <iostream>

int main(int argc, char* argv[])
{
	BoostPerfClient	perfClient("127.0.0.1", 2005, 8);
	std::thread iocThread(&BoostPerfClient::runIoContext, std::ref(perfClient));
	std::thread dispatchSocketsThread(&BoostPerfClient::runSocketsLoop, std::ref(perfClient));

	iocThread.join();
	dispatchSocketsThread.join();
	std::cin.get();
}