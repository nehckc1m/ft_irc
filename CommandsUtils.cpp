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
	
void Server::joinSuccessful(int clientFd, Channel &channel) {
	Reply reply("JOIN", getClientByFd(clientFd));
	Client &client = getClientByFd(clientFd);
	channel.addMember(clientFd);
	sendMessage(clientFd, ":" + client.getNickname() + " JOIN " + channel.getName() + "\r\n");
	if (!channel.getTopic().empty()) {
		sendMessage(clientFd, reply.msg(RPL_TOPIC, channel.getName(), channel.getTopic()));
	}
	std::string names;

	for (unsigned long fd = 0; fd < channel.getMembers().size(); ++fd) {
		if(channel.isOperator(channel.getMembers()[fd])) 
			names += "@";
		names += getClientByFd(channel.getMembers()[fd]).getNickname() + " ";
		
	}
	sendMessage(clientFd,
		":localhost 353 " + client.getNickname() + " = " + channel.getName() + " :" + names + "\r\n"
	);
	sendMessage(clientFd,
		":localhost 366 " + client.getNickname() + " " + channel.getName() + " :End of /NAMES list.\r\n"
	);
}

void Server::user_mode(int clientFd, const std::string &params) {
	Reply reply("MODE", getClientByFd(clientFd));
	std::vector<std::string> args = split_string(params, ' ');
	if (args.size() != 2) {
		sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
		return;
	}
	if (getClientByFd(clientFd).getNickname() != args[0]) {
		sendMessage(clientFd, reply.msg(ERR_USERSDONTMATCH, params));
		return;
	}
}