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
    if (pos != std::string::npos)
		params = cmd.substr(pos + 1);
    cmd = cmd.substr(0, pos);
    // Client &client = getClientByFd(clientFd);
    // if (!check_authentication(client, cmd))
    //     return;
    std::map<std::string, void (Server::*)(int, const std::string&)> commandMap;
    commandMap["JOIN"] = &Server::JOIN;
    commandMap["PART"] = &Server::PART;
    commandMap["PASS"] = &Server::PASS;
    commandMap["NICK"] = &Server::NICK;
    commandMap["USER"] = &Server::USER;
	commandMap["PRIVMSG"] = &Server::PRIVMSG;
	commandMap["MODE"] = &Server::MODE;
	commandMap["TOPIC"] = &Server::TOPIC;
    if (commandMap.find(cmd) != commandMap.end()) {
        (this->*commandMap[cmd])(clientFd, params);
    } else {
		Reply reply(cmd, getClientByFd(clientFd));
		std::cout << reply.msg(ERR_UNKNOWNCOMMAND) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_UNKNOWNCOMMAND));
	}

    std::cout << "Handling command from fd " << clientFd << ": " << cmd << std::endl;
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
    send(clientFd, message.c_str(), message.length(), 0);
}

bool Server::nicknameExists(const std::string &nickname) const
{
	for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		if (it->getNickname() == nickname)
			return true;
	return false;
}
