#include "Server.hpp"
#include "Client.hpp"

Client::Client(int socketFd){
    _fd = socketFd;
    _nickname = "";
	_username = "";
	_realname = "";
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

std::string Client::getUsername() const {
	return _username;
}

std::string Client::getRealname() const {
	return _realname;
}

void Client::setNickname(const std::string &nickname) {
    _nickname = nickname;
}

void Client::setAuthenticate() {
	std::cout << "setAuthenticate" << std::endl;
    _authenticated = true;
}

std::string Client::getNickname() const {
    return _nickname;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}
void Client::setUsername(const std::string &username) {
	_username = username;
}

void Client::setRealname(const std::string &realname) {
	_realname = realname;
}
