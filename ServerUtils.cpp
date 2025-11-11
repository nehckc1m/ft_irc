#include "Server.hpp"

bool Server::_signal = false;

void Server::SignalHandler(int signum) {
    if (signum == SIGINT) {
        std::cout << "\nSIGINT received, shutting down server..." << std::endl;
        _signal = true;
    } else if (signum == SIGQUIT) {
		std::cerr << "\nSIGQUIT received, shutting down server..." << std::endl;
		_signal = true;
	}
}

void Server::processBuffer(int clientFd, char *buffer, int bytes) {
    clientBuffers[clientFd] += std::string(buffer, bytes);
    std::string &clientBuffer = clientBuffers[clientFd];
    size_t pos;

    while ((pos = clientBuffer.find('\n')) != std::string::npos) {
        std::string command = clientBuffer.substr(0, pos);
        clientBuffer.erase(0, pos + 1);
        if (!command.empty() && command[command.size() - 1] == '\r') {
            command.erase(command.size() - 1); // Remove trailing '\r' if present
        }
        // Here you would handle the command, e.g., call server.handleCommand(clientFd, command);
        handleCommand(clientFd, command);
    }
}

Client &Server::getClientByFd(int clientFd) {
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getFd() == clientFd) {
            return clients[i];
        }
    }
    throw std::runtime_error("Client not found.");
}

void Server::handleCommand(int clientFd, const std::string &command) {
    size_t pos;
    pos = command.find(' ');
    std::string cmd = command;
    std::string params = "";
	if (DEBUG == 1) std::cout << GRAY << "[" << clientFd << "]" << P_CYAN << command << RST << std::endl;
    if (pos != std::string::npos)
		params = cmd.substr(pos + 1);
    cmd = cmd.substr(0, pos);
    std::map<std::string, void (Server::*)(int, const std::string&)> commandMap;
    commandMap["JOIN"] = &Server::JOIN;
    commandMap["PART"] = &Server::PART;
    commandMap["PASS"] = &Server::PASS;
    commandMap["NICK"] = &Server::NICK;
    commandMap["USER"] = &Server::USER;
	commandMap["PRIVMSG"] = &Server::PRIVMSG;
	commandMap["MODE"] = &Server::MODE;
	commandMap["TOPIC"] = &Server::TOPIC;
	commandMap["KICK"] = &Server::KICK;
	commandMap["INVITE"] = &Server::INVITE;
	commandMap["PING"] = &Server::PING;
	commandMap["CAP"] = &Server::CAP;
    if (commandMap.find(cmd) != commandMap.end()) {
    	std::cout << "Handling command from fd " << clientFd << ": " << cmd << std::endl;
        (this->*commandMap[cmd])(clientFd, params);
    } else {
		Reply reply(cmd, getClientByFd(clientFd));
		std::cout << reply.msg(ERR_UNKNOWNCOMMAND) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_UNKNOWNCOMMAND));
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

void Server::sendMessage(int clientFd, const std::string &message) {
   
    Client &client = getClientByFd(clientFd);
    try {
        client.queueMessage(message);
    } catch (const std::exception &e) {
        std::cerr << "Client " << clientFd << " disconnected while sending message." << e.what() << std::endl;
        removeClient(clientFd);
        return;
    }
	if (DEBUG == 1) std::cout << GRAY << "[->" << clientFd << "]" P_CYAN << message << RST <<std::endl;
    flushSendBuffer(client);
}

void Server::flushSendBuffer(Client &client) {
    std::string &buffer = client.getSendBuffer();
    while (!buffer.empty()) {
        ssize_t bytesSent = send(client.getFd(), buffer.c_str(), buffer.size(), 0);
        if (bytesSent > 0){
            buffer.erase(0, bytesSent);
        } else if (bytesSent == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                enablePollOut(client.getFd());
                return;
            } else {
                std::cerr << "Error sending message to client " << client.getFd() << std::endl;
                removeClient(client.getFd());
                return;
            }
        }
    }
    disablePollOut(client.getFd());
}

bool Server::nicknameExists(const std::string &nickname) const
{
	for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		if (it->getNickname() == nickname)
			return true;
	return false;
}

void Server::enablePollOut(int clientFd) {
    for (size_t i = 0; i < poll_fds.size(); ++i) {
        if (poll_fds[i].fd == clientFd) {
            poll_fds[i].events |= POLLOUT;
            return;
        }
    }
}

void Server::disablePollOut(int clientFd) {
    for (size_t i = 0; i < poll_fds.size(); ++i) {
        if (poll_fds[i].fd == clientFd) {
            poll_fds[i].events &= ~POLLOUT;
            return;
        }
    }
}

std::vector<std::string> split_string(const std::string s, char sep)
{
	std::vector<std::string> params;
	std::istringstream ss(s);
	std::string buffer;
	while (std::getline(ss, buffer, sep))
		params.push_back(buffer);
	return params;
}
