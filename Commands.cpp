#include "Server.hpp"
#include "Reply.hpp"
#include <cstdlib>

void Server::PASS(int clientFd, const std::string &params) {
    
    Client &client = getClientByFd(clientFd);
	if (params.empty())
			return sendMessage(clientFd, ":localhost 461 PASS :Not enough parameters\r\n");

    if (client.isAuthenticated()) {
        return sendMessage(clientFd, ":localhost 462 :You may not reregister\r\n");
    }
	if (params == _password) {
        client.setAuthenticate();
        std::cout << "Client " << clientFd << " authenticated successfully." << std::endl;
		return;
    }
    else {
        std::cout << "Client " << clientFd << " provided incorrect password." << std::endl;
        sendMessage(clientFd, ":localhost 464 :Password incorrect\r\n");
    }
}

void Server::JOIN(int clientFd, const std::string &params) {
	Client &client = getClientByFd(clientFd);

	Reply r("JOIN", client);

	if (!client.isRegistered())
		return sendMessage(clientFd, r.msg(ERR_NOTREGISTERED));

	if (params.empty()) {
        sendMessage(clientFd, ":localhost 461 JOIN :Not enough parameters\r\n");
        return;
    }
    std::string channelName = params;
    size_t spacePos = params.find(' ');
    if (spacePos != std::string::npos)
        channelName = params.substr(0, spacePos);
    if (channelName[0] != '#') {
        sendMessage(clientFd, ":localhost 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }
    Channel *channel = NULL;
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].getName() == channelName) {
            channel = &channels[i];
            break;
        }
    }
    if (!channel) {
        std::cout << "Channel " << channelName << " created." << std::endl;
        channels.push_back(Channel(channelName));
        channels.back().addMember(clientFd);
        channels.back().addOperator(clientFd);
        channel = &channels.back();
		return joinSuccessful(clientFd, *channel);
    } 
    if (channel->isMember(clientFd)) {
        sendMessage(clientFd, ":localhost 443 " + client.getNickname() + " " + channelName + " :You're already on that channel\r\n");
		return;
    }
    if (channel->getProtected()){
        std::cout << "Channel " << params << " is protected. Checking password." << std::endl;
        size_t spacePos = params.find(' ');
        std::string channelName = params.substr(0, spacePos);
        std::string password = params.substr(spacePos + 1); 
        if (password != channel->getPassword())
            return sendMessage(clientFd, ":localhost 475 " + client.getNickname() + " " + channelName + " :Cannot join protected channel\r\n");
    }
    if (channel->getInviteOnly() && !channel->isInvited(clientFd)) {
        std::cout << "Channel " << params << " is invite-only. Checking invitation." << std::endl;
        return sendMessage(clientFd, ":localhost 473 " + client.getNickname() + " " + channelName + " :Cannot join invite-only channel without invitation\r\n");
    }
	if (channel->getUserLimit() > 0 && channel->getMembers().size() >= channel->getUserLimit()) {
		std::cout << "Channel " << params << " has reached its user limit." << std::endl;
		return sendMessage(clientFd, ":localhost 471 " + client.getNickname() + " #" + channelName + " :Cannot join channel, user limit reached\r\n");
	}
	channel->addMember(clientFd);
	joinSuccessful(clientFd, *channel);
}

void Server::PRIVMSG(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, ":localhost 411 :No recipient given (PRIVMSG)\r\n");
        return;
    }
    size_t spacePos = params.find(' ');
    if (spacePos == std::string::npos) {
        sendMessage(clientFd, ":localhost 412 :No text to send\r\n");
        return;
    }
    std::string target = params.substr(0, spacePos);
    std::string message = params.substr(spacePos + 1);
    
    if (message.empty()) {
        sendMessage(clientFd, ":localhost 412 :No text to send\r\n");
        return;
    }
    if (message[0] == ':') {
        message = message.substr(1);
    }
    Client &client = getClientByFd(clientFd);
    // Target is a channel
    if (target[0] == '#') {
        MSG_CHANNEL(clientFd, target, message);
        return;
    }
    // Target is a user
    int targetFd = -1;
    std::string targetNick = "";
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getNickname() == target) {
            targetFd = clients[i].getFd();
            targetNick = clients[i].getNickname();
            break;
        }
    }
    if (targetFd == -1) {
        sendMessage(clientFd, ":localhost 401 " + client.getNickname() + " " + target + " :No such nick/channel\r\n");
        return;
    }
    std::cout << "Message sent to " << target << std::endl;
    sendMessage(targetFd, ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n");
}

void Server::MSG_CHANNEL(int clientFd, const std::string &channelName, const std::string &message) {
    std::cout << "Client " << clientFd << " is sending message to channel " << channelName << ": " << message << std::endl;
    
    Client &client = getClientByFd(clientFd);
    size_t i;
    for (i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            break;
        }
    }
    
    if (i >= channels.size()) {
        sendMessage(clientFd, ":localhost 403 " + channelName + " :No such channel\r\n");
        return;
    }
    if (!isPartOfChannel(clientFd, channelName)) {
        sendMessage(clientFd, ":localhost 404 " + channelName + " :Cannot send to channel\r\n");
        return;
    }
    
    if (message.empty()) {
        sendMessage(clientFd, ":localhost 412 :No text to send\r\n");
        return;
    }
    const std::vector<int> &members = channels[i].getMembers();
    for (size_t j = 0; j < members.size(); ++j) {
        if (members[j] != clientFd) {
            sendMessage(members[j], ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + channelName + " :" + message + "\r\n");
        }
    }
}

void Server::MODE(int clientFd, const std::string &params) {
    Reply reply("MODE", getClientByFd(clientFd));
    if (params.empty()) {
        sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
        return;
    }
    
    if (params[0] != '#') {
        user_mode(clientFd, params);
        return;
    }
    
    std::string channelName;
    std::string modeStr;
    std::vector<std::string> modeParams;
    
    std::istringstream iss(params);
    iss >> channelName >> modeStr;
    
    std::string param;
    while (iss >> param) {
        modeParams.push_back(param);
    }
    Client &client = getClientByFd(clientFd);
    size_t i = 0;
    for (i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            break;
        }
    }
    
    if (i == channels.size()) {
        sendMessage(clientFd, ":localhost 403 " + channelName + " :No such channel\r\n");
        return;
    }
    
    Channel &channel = channels[i];
 
    if (!isPartOfChannel(clientFd, channelName)) {
        sendMessage(clientFd, ":localhost 442 " + channelName + " :You're not on that channel\r\n");
        return;
    }
 
    if (modeStr.empty()) {
        std::string modes = "+";
        if (channel.getTopicRestricted()) modes += "t";
        if (channel.getInviteOnly()) modes += "i";
        if (channel.getProtected()) modes += "k";
        if (channel.getUserLimit() > 0) modes += "l";
        sendMessage(clientFd, ":localhost 324 " + client.getNickname() + " " + channelName + " " + modes + "\r\n");
        return;
    }
    
    if (!channel.isOperator(clientFd)) {
        sendMessage(clientFd, ":localhost 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
    
    bool adding = (modeStr[0] == '+');
    std::string appliedModes = modeStr.substr(0, 1);
    size_t paramIndex = 0;
    
    for (size_t j = 1; j < modeStr.size(); ++j) {
        char mode = modeStr[j];
        
        if (mode == 't') {
            channel.toggleTopic();
            appliedModes += mode;
        }
        else if (mode == 'o') {
            if (paramIndex >= modeParams.size()) {
                sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
                return;
            }
            std::string targetNick = modeParams[paramIndex++];
            int targetFd = getClientFdByNickname(targetNick);
            if (targetFd == -1) {
                sendMessage(clientFd, ":localhost 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
                return;
            }
            if (adding) {
                channel.addOperator(targetFd);
            } else {
                channel.removeOperator(targetFd);
            }
            appliedModes += mode;
        }
        else if (mode == 'i') {
            channel.setInviteOnly(adding);
            appliedModes += mode;
        }
        else if (mode == 'k') {
            if (adding) {
                if (paramIndex >= modeParams.size()) {
                    sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
                    return;
                }
                std::string password = modeParams[paramIndex++];
                channel.setPassword(password);
            } else {
                channel.removePassword();
            }
            appliedModes += mode;
        }
        else if (mode == 'l') {
            if (adding) {
                if (paramIndex >= modeParams.size()) {
                    sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
                    return;
                }
                std::string limitStr = modeParams[paramIndex++];
                int limit = atoi(limitStr.c_str());
                if (limit <= 0) {
                    sendMessage(clientFd, ":localhost 501 " + client.getNickname() + " :Invalid user limit\r\n");
                    return;
                }
                channel.setUserLimit(limit);
            } else {
                channel.setUserLimit(0);
            }
            appliedModes += mode;
        }
        else {
            sendMessage(clientFd, ":localhost 501 " + client.getNickname() + " :Unknown MODE flag\r\n");
            return;
        }
    }
    
    std::string modeReply = appliedModes;
    for (size_t k = 0; k < paramIndex; ++k) {
        modeReply += " " + modeParams[k];
    }

    sendMessage(clientFd, ":localhost MODE " + channelName + " " + modeReply + "\r\n");

    const std::vector<int> &members = channel.getMembers();
    for (size_t k = 0; k < members.size(); ++k) {
        sendMessage(members[k], ":" + client.getNickname() + "!" + client.getUsername() + "@localhost MODE " + channelName + " " + modeReply + "\r\n");
    }
}

void Server::TOPIC(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, ":localhost 461 TOPIC :Not enough parameters\r\n");
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

    Channel *channel = NULL;
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            channel = &channels[i];
            break;
        }
    }

    if (!channel) {
        sendMessage(clientFd, ":localhost 403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!isPartOfChannel(clientFd, channelName)) {
        sendMessage(clientFd, ":localhost 442 " + channelName + " :You're not on that channel\r\n");
        return;
    }

    Client &client = getClientByFd(clientFd);

    if (topic.empty()) {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty()) {
            sendMessage(clientFd, ":localhost 331 " + client.getNickname() + " " + channelName + " :No topic is set\r\n");
        } else {
            sendMessage(clientFd, ":localhost 332 " + client.getNickname() + " " + channelName + " :" + currentTopic + "\r\n");
        }
    } else {
        bool topicRestricted = channel->getTopicRestricted();
        bool isOperator = channel->isOperator(clientFd);

        if (topicRestricted && !isOperator) {
            sendMessage(clientFd, ":localhost 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
            return;
        }

        channel->setTopic(topic);

        sendMessage(clientFd, ":localhost TOPIC " + channelName + " :" + topic + "\r\n");

        const std::vector<int> &members = channel->getMembers();
        for (size_t i = 0; i < members.size(); ++i) {
            sendMessage(members[i], ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channelName + " :" + topic + "\r\n");
        }
    }
}

void Server::NICK(int clientFd, const std::string &params) {

    Client &client = getClientByFd(clientFd);
    if (!client.isAuthenticated()) {
        return sendMessage(clientFd, ":localhost 464 :Password incorrect\r\n");
    }

	Reply reply("NICK", getClientByFd(clientFd));
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

void Server::USER(int clientFd, const std::string &params) {
    Client &client = getClientByFd(clientFd);
    if (client.getNickname().empty()) {
        return sendMessage(clientFd, ":localhost 451 :You have not registered\r\n");
    }

	Reply reply("USER", client);

	std::vector<std::string> args = split_string(params, ' ');

	if (args.size() < 4) {
		std::cout << reply.msg(ERR_NEEDMOREPARAMS) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
		return;
	}

	if (!client.getUsername().empty() || !client.getRealname().empty()) {
		std::cout << reply.msg(ERR_ALREADYREGISTRED) << std::endl;
		sendMessage(clientFd, reply.msg(ERR_ALREADYREGISTRED));
		return;
	}

	std::string realname = args[3];
    realname.erase(realname.find_last_not_of("\n\r") + 1);
	if (realname[0] == ':')
		realname.erase(0,1);
	else
	{
		std::string msg("Error: malformed USER command. Realname must start with ':'");
		std::cout << msg << std::endl;
		sendMessage(clientFd, msg + ":localhost 461 USER :Not enough parameters\r\n");
		return;
	}	
	client.setUsername(args[0]);
	client.setRealname(realname);
	client.setRegistered();
	std::cout << "Client " << clientFd << " registered as " << client.getNickname() << " (" << client.getUsername() << ")" << std::endl;
	// sendMessage(clientFd, reply.msg(RPL_WELCOME));
	sendMessage(clientFd, ":localhost 001 " + client.getNickname() + ORANGE " :Welcome to the FT_IRC server " RST + client.getNickname() + "!" + client.getUsername() + "@localhost\r\n");
}
	
void Server::PART(int clientFd, const std::string &params) {
    std::cout << "Client " << clientFd << " is attempting to part channel with params: " << params << std::endl;

    if (params.empty()) {
        sendMessage(clientFd, "ERROR :No channel specified\r\n");
        return;
    }
    if (params[0] != '#') {
        sendMessage(clientFd, "ERROR :Invalid channel name\r\n");
        return;
    }
	// Check if client is part of the channel
	if (!isPartOfChannel(clientFd, params)) {
		sendMessage(clientFd, "ERROR :You are not a member of channel " + params + "\r\n");
		return;
	}

    for (size_t i = 0; i < channels.size(); ++i) {
        std::cout << "Checking existing channel: " << channels[i].getName() << std::endl;
        if (channels[i].getName() == params) {
            channels[i].removeMember(clientFd);
			channels[i].removeOperator(clientFd);
			channels[i].removeInvitedMember(clientFd);
            std::cout << "Client " << clientFd << " parted from existing channel: " << params << std::endl;
			sendMessage(clientFd, ":" + getClientByFd(clientFd).getNickname() + "!~" + getClientByFd(clientFd).getUsername() + "@localhost PART " + channels[i].getName() + "\r\n");
            return;
        }
    }
    sendMessage(clientFd, "ERROR :No such channel\r\n");
    return;
}

void Server::KICK(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, ":localhost 461 KICK :Not enough parameters\r\n");
        return;
    }
    Client &client = getClientByFd(clientFd);
    std::vector<std::string> tokens = split_string(params, ' ');
    if (tokens.size() < 2) {
        sendMessage(clientFd, ":localhost 461 KICK :Not enough parameters\r\n");
        return;
    }
    std::string channelName = tokens[0];
    std::string nickToKick = tokens[1];
    std::string reason = "";
    if (tokens.size() > 2) {
        reason = tokens[2];
    }
    size_t i;
    for (i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName)
            break;
    }
    if (i >= channels.size()) {
        sendMessage(clientFd, ":localhost 403 " + channelName + " :No such channel\r\n");
        return;
    }
    if (!channels[i].isMember(clientFd)) {
        sendMessage(clientFd, ":localhost 442 " + channelName + " :You're not on that channel\r\n");
        return;
    }
    if (!channels[i].isOperator(clientFd)) {
        sendMessage(clientFd, ":localhost 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
    size_t j;
    int kickFd = -1;
    for (j = 0; j < clients.size(); ++j) {
        if (clients[j].getNickname() == nickToKick) {
            kickFd = clients[j].getFd();
            break;
        }
    }
    if (kickFd == -1) {
        sendMessage(clientFd, ":localhost 401 " + client.getNickname() + " " + nickToKick + " :No such nick/channel\r\n");
        return;
    }
    if (!channels[i].isMember(kickFd)) {
        sendMessage(clientFd, ":localhost 441 " + nickToKick + " " + channelName + " :They are not on that channel\r\n");
        return;
    }
    channels[i].removeMember(kickFd);
    std::string kickMessage;
    if (!reason.empty()) {
        kickMessage = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channelName + " " + nickToKick + " :" + reason + "\r\n";
    } else {
        kickMessage = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channelName + " " + nickToKick + "\r\n";
    }
    const std::vector<int> &members = channels[i].getMembers();
    for (size_t k = 0; k < members.size(); ++k) {
        sendMessage(members[k], kickMessage);
    }
    sendMessage(kickFd, kickMessage);
}

void Server::INVITE(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, ":localhost 461 INVITE :Not enough parameters\r\n");
        return;
    }
    Client &client = getClientByFd(clientFd);
    std::vector<std::string> tokens = split_string(params, ' ');
    if (tokens.size() < 2) {
        sendMessage(clientFd, ":localhost 461 INVITE :Not enough parameters\r\n");
        return;
    }
    std::string nickToInvite = tokens[0];
    std::string channelName = tokens[1];
    std::cout << "INVITE: Client " << clientFd << " is inviting " << nickToInvite 
              << " to channel " << channelName << std::endl;
    size_t i;
    for (i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName)
            break;
    }
    if (i >= channels.size()) {
        sendMessage(clientFd, ":localhost 403 " + channelName + " :No such channel\r\n");
        return;
    }
    if (isPartOfChannel(clientFd, channelName) == false) {
        sendMessage(clientFd, ":localhost 442 " + channelName + " :You're not on that channel\r\n");
        return;
    }
    size_t j;
    int inviteFd = -1;
    for (j = 0; j < clients.size(); ++j) {
        if (clients[j].getNickname() == nickToInvite) {
            inviteFd = clients[j].getFd();
            break;
        }
    }
    if (inviteFd == -1) {
        sendMessage(clientFd, ":localhost 401 " + client.getNickname() + " " + nickToInvite + " :No such nick/channel\r\n");
        return;
    }
    if (channels[i].isMember(inviteFd)) {
        sendMessage(clientFd, ":localhost 443 " + client.getNickname() + " " + nickToInvite + " " + channelName + " :is already on channel\r\n");
        return;
    }
    channels[i].addInvitedMember(inviteFd);
    sendMessage(clientFd, ":localhost 341 " + client.getNickname() + " " + nickToInvite + " " + channelName + "\r\n");
    sendMessage(inviteFd, ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + nickToInvite + " " + channelName + "\r\n");
}

void Server::PING(int clientFd, const std::string &params) {
	sendMessage(clientFd, ":localhost PONG localhost :" + params + "\r\n");
}

void Server::CAP(int clientFd, const std::string &params) {
	(void) clientFd;
	(void) params;
}

void Server::QUIT(int clientFd, const std::string &params) {
	std::cout << "Server::QUIT called" << std::endl;
	Client& c = getClientByFd(clientFd);
	std::string quit_message = (!params.empty() && params[0] == ':') ? params : ":Client Quit";
	quit_message = ":" + c.getNickname() + "!~" + c.getUsername() + "@localhost QUIT " + quit_message + "\r\n";
	for (std::vector<Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
		if (it->isMember(clientFd)) {
			for (std::vector<int>::const_iterator itm = it->getMembers().begin(); itm != it->getMembers().end(); ++itm)
				if (*itm != clientFd) sendMessage(*itm, quit_message);
				//	sendMessage(*itm, ":"+c.getNickname()+"!~"+ c.getUsername() + "@localhost QUIT " + quit_message + "\r\n");
		}
	}
	removeClient(clientFd);
}
