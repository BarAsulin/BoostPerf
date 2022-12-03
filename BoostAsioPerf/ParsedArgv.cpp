#include "ParsedArgv.h"
#include <iostream>
#include "cassert"

ParsedArgv::ParsedArgv(int argc, char* argv[])
{
	for (size_t i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--server") == 0)
		{
			m_IsServer = true;
			assert((m_IsClient == false) && ("can't be both client and server"));
		}
		else if (strcmp(argv[i], "--client") == 0)
		{
			m_IsClient = true;
			++i;
			assert((i < argc) && "Missing arguments");
			m_serverHostname = argv[i];
			assert((m_IsServer == false) && ("can't be both client and server"));
		}
		else if (strcmp(argv[i], "--port") == 0)
		{
			++i;
			assert((i < argc) && "Missing arguments");
			m_port = std::stoi(argv[i]);
		}
		else if (strcmp(argv[i], "--sockets") == 0)
		{
			++i;
			assert((i < argc) && "Missing arguments");
			m_sockets = std::stoi(argv[i]);
		}
		else if (strcmp(argv[i], "--TLS") == 0)
		{
			m_usingSSL = true;
		}
	}
}

void ParsedArgv::printUsage()
{
	std::cerr << "Usage: [ --server ] | [ --client <ServerIp> ] [ --port <port> ] [--TLS ](uses TLS1.2) [ --sockets <numOfSockets> ](8 sockets are used by default)\n";
	std::cerr << "Example:\n";
	std::cerr << "Client: --client 192.168.111.20 --port 2005 --sockets 20\n";
	std::cerr << "Server: --server --port 2005 --sockets 20\n";
}
