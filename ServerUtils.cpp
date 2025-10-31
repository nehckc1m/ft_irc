#include "Server.hpp"

bool Server::_signal = false;

void Server::SignalHandler(int signum) {
    if (signum == SIGINT) {
        std::cout << "\nSIGINT received, shutting down server..." << std::endl;
        _signal = true;
    }
}

void Server::closeFds() {
    for (size_t i = 0; i < clients.size(); i++)
    {
        std::cout << "Client <" << clients[i].getFd() << "> disconnected." << std::endl;
        close(clients[i].getFd());
    }
    if (_servSocket != -1)
    {
        
        close(_servSocket);
    }
}

int Server::getPort() const {
    return _port;
}