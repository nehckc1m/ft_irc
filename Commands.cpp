#include "Server.hpp"


void Server::PASS(int clientFd, const std::string &params) {
    if (params == _password) {
        clients[clientFd].setAuthenticate();
        std::cout << "Client " << clientFd << " authenticated successfully." << std::endl;
        sendMessage(clientFd, "Welcome to the IRC server!\r\n");
    }
    else {
        std::cout << "Client " << clientFd << " provided incorrect password." << std::endl;
        sendMessage(clientFd, "ERROR :Password incorrect\r\n");
    }
}

