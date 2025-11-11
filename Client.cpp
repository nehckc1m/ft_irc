#include "Server.hpp"
#include "Client.hpp"

Client::Client(int socketFd){
    _fd = socketFd;
    _nickname = "";
	_username = "";
	_realname = "";
    _authenticated = false;
	_registered = false;
}

/*Client& Client::operator=(const Client &other) {
    if (this != &other) {
        _fd = other._fd;
        _nickname = other._nickname;
        _username = other._username;
        _channels = other._channels;
        _sendBuffer = other._sendBuffer;
        _authenticated = other._authenticated;
    }
    return *this;
}*/

Client::~Client() {
    // Clean up resources if needed
}

int Client::getFd() const {
    return _fd;
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
    _authenticated = true;
}

void Client::setRegistered() {
    _registered = true;
}

std::string Client::getNickname() const {
    return _nickname;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}

bool Client::isRegistered() const {
    return _registered;
}

void Client::setUsername(const std::string &username) {
	_username = username;
}

void Client::queueMessage(const std::string &msg) {
    const size_t MAX_BUFFER = 65536; // 64KB
    
    if (_sendBuffer.size() + msg.size() > MAX_BUFFER) {
        std::cout << "Client " << getFd() << " send buffer overflow!" << std::endl;
        throw std::runtime_error("Buffer overflow");
    }
    _sendBuffer += msg;
}

std::string &Client::getSendBuffer() {
    return _sendBuffer;
}

void Client::setRealname(const std::string &realname) {
	_realname = realname;
}
