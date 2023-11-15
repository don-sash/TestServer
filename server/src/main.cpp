#include <iostream>

#include "server.hpp"

int main(int argc, char *argv[])
{
	uint16_t port         = 5071;
	uint32_t treads_count = 10;

	std::cout << "************* Start TCP server *************" << std::endl;
	std::cout << "Usage: Run with args: server <port> <threads count> " << std::endl;
	std::cout << "  or with defaults port = " << port << " threads count = " << treads_count << std::endl << std::endl;

	if (argc > 1)
	{
		std::istringstream iss(argv[1]);
		if (!(iss >> port))
		{
			std::cout << "Wrong port argument!";
			return 1;
		}
	}

	if (argc > 2)
	{
		std::istringstream iss(argv[2]);
		if (!(iss >> treads_count))
		{
			std::cout << "Wrong threads count argument!";
			return 1;
		}
	}

	try
	{
		asio::io_context io_context;
		Server server(io_context, port);
		server.Run();
	}
	catch (std::exception &e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}