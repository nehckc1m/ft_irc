#include "Server.hpp"
#include "Reply.hpp"

void Server::PASS(int clientFd, const std::string &params) {
    
    Client &client = getClientByFd(clientFd);
    if (client.isAuthenticated()) {
        sendMessage(clientFd, "ERROR :You are already authenticated\r\n");
        return;
    }

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

void Server::PRVMSG(int clientFd, const std::string &params) {

    if (params.empty()) {
        sendMessage(clientFd, "ERROR :No target specified\r\n");
        return;
    }
    size_t spacePos = params.find(' ');
    if (spacePos == std::string::npos) {
        sendMessage(clientFd, "ERROR :No message specified\r\n");
        return;
    }
    std::string target = params.substr(0, spacePos);
    // Target is a channel
    if (target[0] == '#') {
        std::string message = params.substr(spacePos + 1);
        if (message.empty()) {
            sendMessage(clientFd, "ERROR :No message specified\r\n");
            return;
        }
        MSG_CHANNEL(clientFd, target, message);
        return;
    }
    // Target is a user
    std::string message = params.substr(spacePos + 1);
    bool targetFound = false;
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getNickname() == target) {
            targetFound = true;
            break;
        }
    }
    if (targetFound) {
        sendMessage(clientFd, "Message sent to " + target + "\r\n");
    } else {
        sendMessage(clientFd, "ERROR :No such nick/channel\r\n");
    }
}

void Server::MSG_CHANNEL(int clientFd, const std::string channelName,const std::string message) {
    
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            if (channels[i].isMember(clientFd)) {
                const std::vector<int> &members = channels[i].getMembers();
                for (size_t j = 0; j < members.size(); ++j) {
                    if (members[j] != clientFd) {
                        sendMessage(members[j], "PRIVMSG " + channelName + ": " + message + "\r\n");
                    }
                }
            } else {
                sendMessage(clientFd, "ERROR :You are not a member of channel " + channelName + "\r\n");
            }
            return;
        }
    }
    sendMessage(clientFd, "ERROR :No such channel\r\n");
}

void Server::MODE(int clientFd, const std::string &params) {
    // Placeholder for MODE command handling logic
    sendMessage(clientFd, "MODE command received with params: " + params + "\r\n");
}

void Server::TOPIC(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, "ERROR :No channel specified\r\n");
        return;
    }
    std::string channelName;
    size_t spacePos = params.find(' ');
    if (spacePos == std::string::npos) {
        channelName = params;
    } else {
        channelName = params.substr(0, spacePos);
    }
    std::string topic;
    if (spacePos != std::string::npos) {
        topic = params.substr(spacePos + 1);
    }

    if (topic.empty()) {
        // Get topic
        for (size_t i = 0; i < channels.size(); ++i) {
            if (channels[i].getName() == channelName) {
                if (isPartOfChannel(clientFd, channelName))
                    return sendMessage(clientFd, "Topic for " + channelName + ": " + channels[i].getTopic() + "\r\n");
                else
                    return sendMessage(clientFd, "ERROR :You are not a member of channel " + channelName + "\r\n");
            }
        }
        return sendMessage(clientFd, "ERROR :No such channel\r\n");
    }
    else {
        // Set topic
        for (size_t i = 0; i < channels.size(); ++i) {
            if (channels[i].getName() == channelName) {
                if (!isPartOfChannel(clientFd, channelName)) {
                    return sendMessage(clientFd, "ERROR :You are not a member of channel " + channelName + "\r\n");
                }
                if (!channels[i].getTopicRestricted()) {
                    channels[i].setTopic(topic);
                    return sendMessage(clientFd, "Topic for " + channelName + " set to: " + topic + "\r\n");
                }
                else if (channels[i].isOperator(clientFd) && channels[i].getTopicRestricted()) {
                    channels[i].setTopic(topic);
                    sendMessage(clientFd, "Topic for " + channelName + " set to: " + topic + "\r\n");
                    return;
                } else {
                    return sendMessage(clientFd, "ERROR :You are not authorized to set the topic for " + channelName + "\r\n");
                }
            }
        }
        return sendMessage(clientFd, "ERROR :No such channel\r\n");
    }
}
void Server::NICK(int clientFd, const std::string &params) {
	std::cout << "Client " << clientFd << " is attempting to register NICK command with params: " << params << std::endl;

	Reply reply("NICK");

	if (params.empty()) {
		std::cout << reply.msg(ERR_NONICKNAMEGIVEN) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_NONICKNAMEGIVEN));
		return;
	}
	// Check if not correct nickname
	if (params.find(' ') != std::string::npos) {
		std::cout << reply.msg(ERR_ERRONEUSNICKNAME) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_ERRONEUSNICKNAME));
		return;
	}
	// Check if nickname already exists
	if (nicknameExists(params)) {
		std::cout << reply.msg(ERR_NICKNAMEINUSE) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_NICKNAMEINUSE));
		return;
	}

	// Setting nickname
	getClientByFd(clientFd).setNickname(params);
}

// Returns vector of strings created by splitting the string by separator sep
std::vector<std::string> split_string(const std::string s, char sep)
{
	std::vector<std::string> params;
	std::istringstream ss(s);
	std::string buffer;
	while (std::getline(ss, buffer, sep))
		params.push_back(buffer);
	return params;
}

void Server::USER(int clientFd, const std::string &params) {
	std::cout << "Client " << clientFd << " is attempting to register USER with params: " << params << std::endl;

	Reply reply("USER");

	if (params.empty()) {
		std::cout << reply.msg(ERR_NEEDMOREPARAMS) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
		return;
	}

	std::vector<std::string> args = split_string(params, ' ');

	if (args.size() < 4) {
		std::cout << reply.msg(ERR_NEEDMOREPARAMS) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
		return;
	}

	Client &client = getClientByFd(clientFd);

	// Check if nickname already exists
	if (client.isAuthenticated()) {
		std::cout << reply.msg(ERR_ALREADYREGISTRED) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_ALREADYREGISTRED));
		return;
	}

	client.setUsername(args[0]);
	client.setHostname(args[1]);
	client.setServername(args[2]);

    std::string realname = args[3];
    realname.erase(realname.find_last_not_of("\n\r") + 1);
	if (realname[0] == ':')
		realname.erase(0,1);
	else
	{
		std::string msg("Error: malformed USER command. Realname must start with ':'");
		std::cout << msg << std::endl;
		sendMessage(clientFd, msg + "\r\n");
		return;
	}	
	client.setRealname(realname);
	client.setAuthenticate();
}
	
