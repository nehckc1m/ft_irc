#include "Server.hpp"
#include "Client.hpp"

Client::Client(int socketFd){
    _fd = socketFd;
    _authenticated = false;
    _nickname = "";
    _authenticated = false;
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

std::string Client::getUser() const {
    return _username;
}

void Client::setNickname(const std::string &nickname) {
    _nickname = nickname;
}

void Client::setAuthenticate() {
    _authenticated = true;
}

std::string Client::getNickname() const {
    return _nickname;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}