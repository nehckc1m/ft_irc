#include "Server.hpp"
#include "Client.hpp"

Client::Client(int socketFd)
    : _fd(socketFd), _nickname(""), _username(""), _authenticated(false) {
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

void Client::setNickname(const std::string &nickname) {
    _nickname = nickname;
}

void Client::setAuthenticate() {
    _authenticated = true;
}