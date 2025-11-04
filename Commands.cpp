#include "Server.hpp"


void Server::PASS(int clientFd, const std::string &params) {
    
    Client &client = getClientByFd(clientFd);
    
    if (params == _password) {
        client.setAuthenticate();
        std::cout << "Client " << clientFd << " authenticated successfully." << std::endl;
        sendMessage(clientFd, "Welcome to the IRC server!\r\n");
    }
    else {
        std::cout << "Client " << clientFd << " provided incorrect password." << std::endl;
        sendMessage(clientFd, "ERROR :Password incorrect\r\n");
    }
}

void Server::JOIN(int clientFd, const std::string &params) {
    std::cout << "Client " << clientFd << " is attempting to join channel with params: " << params << std::endl;
    if (params.empty()) {
        sendMessage(clientFd, "ERROR :No channel specified\r\n");
        return;
    }
    if (params[0] != '#') {
        sendMessage(clientFd, "ERROR :Invalid channel name\r\n");
        return;
    }

    for (size_t i = 0; i < channels.size(); ++i) {
        std::cout << "Checking existing channel: " << channels[i].getName() << std::endl;
        if (channels[i].getName() == params) {
            channels[i].addMember(clientFd);
            std::cout << "Client " << clientFd << " joined existing channel: " << params << std::endl;
            sendMessage(clientFd, "Joined channel " + params + "\r\n");
            return;
        }
    }
    Channel newChannel(params);
    std::cout << "Creating new channel: " << params << std::endl;
    newChannel.addMember(clientFd);
    newChannel.addOperator(clientFd);
    channels.push_back(newChannel);
    sendMessage(clientFd, "Joined channel " + params + "\r\n");
    return;

}
