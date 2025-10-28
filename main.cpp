#include "Server.hpp"


int main (int ac, char **av) {
    if (ac != 3) {
        std::cerr <<"Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    try{
        Server server(std::atoi(av[1]), av[2]);
        server.createSocket();
        //signal(SIGINT, Server::SignalHandler);
        server.run();
        
    }catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    
    }
    return 0;
}