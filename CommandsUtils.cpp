#include "Server.hpp"


bool Server::isPartOfChannel(int clientFd, const std::string &channelName) {
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            return channels[i].isMember(clientFd);
        }
    }
    return false;
}

int Server::check_authentication(Client &client, std::string cmd) {
    if (!client.isAuthenticated() && cmd != "PASS") {
        sendMessage(client.getFd(), "ERROR :You must authenticate first\r\n");
        return 0;
    }
   /* if (client.getUsername().empty() && cmd != "USER" && client.isAuthenticated()) {
        sendMessage(client.getFd(), "ERROR :You must set your username first\r\n");
        return 0;
    }*/
    return 1;
}

int Server::getFdByNickname(const std::string &nickname) {
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getNickname() == nickname) {
            return clients[i].getFd();
        }
    }
    return -1;
}