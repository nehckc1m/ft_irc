#include "Server.hpp"

void check_args(int ac, char **av)
{
    if (ac != 3)
		throw std::invalid_argument("incorrect number of arguments \nUsage: ./ircserv <port> <password>");
	const int port = std::atoi(av[1]);
	if (port < 1024 || port > 65535)
		throw std::invalid_argument("incorrect port number");
}

int main (int ac, char **av) {
	try {
		check_args(ac, av);
        Server server(std::atoi(av[1]), av[2]);
        server.createSocket();
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        server.run();
        
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}