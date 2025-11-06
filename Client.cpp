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

std::string Client::getUsername() const {
	return _username;
}

std::string Client::getRealname() const {
	return _realname;
}

std::string Client::getHostname() const
{
	return _hostname;
}

std::string Client::getServername() const
{
	return _servername;
}

void Client::setNickname(const std::string &nickname) {
    _nickname = nickname;
}

void Client::setAuthenticate() {
	std::cout << "setAuthenticate";
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

void Client::setHostname(const std::string &hostname)
{
	_hostname = hostname;
}

void Client::setServername(const std::string &servername)
{
	_servername = servername;
}
