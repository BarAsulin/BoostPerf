#pragma once
#include <string>

struct ParsedArgv
{
	ParsedArgv(int argc, char* argv[]);
	void printUsage();

	std::string m_serverHostname;
	bool m_IsServer {false};
	bool m_IsClient {false};
	int m_port{};
	int m_sockets{8};
};

