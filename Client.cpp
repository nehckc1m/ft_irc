#include "Server.hpp"
#include "Client.hpp"

Client::Client(int socketFd)
    : _fd(socketFd), _nickname(""), _username(""), _isAuthenticated(false) {
    // Additional initialization if needed
}

Client::~Client() {
    // Clean up resources if needed
}

int Client::getFd() const {
    return _fd;
}

std::string Client::getIpAddress() const {
    return _ipAddress;
}