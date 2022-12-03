#ifndef PARSED_ARGV
#define PARSED_ARGV
#include <string>

struct ParsedArgv
{
	ParsedArgv(int argc, char* argv[]);
	static void printUsage();

	std::string m_serverHostname;
	bool m_IsServer{ false };
	bool m_IsClient{ false };
	bool m_usingSSL{ false };
	int m_port{};
	int m_sockets{ 8 };
};


#endif // !PARSED_ARGV