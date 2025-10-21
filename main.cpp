#include "Server.hpp"


int main (int ac, char **av) {
    if (ac != 3) {
        std::cerr <<"Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    
}