#include "Runner.h"

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		ParsedArgv::printUsage();
	}
	ParsedArgv args{ argc,argv };
	Runner::run(args);
	return 0;
}

