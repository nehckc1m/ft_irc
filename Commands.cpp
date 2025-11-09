#include "Server.hpp"
#include "Reply.hpp"
#include <cstdlib>

void Server::PASS(int clientFd, const std::string &params) {
    
    Client &client = getClientByFd(clientFd);
    if (client.isAuthenticated()) {
        return sendMessage(clientFd, "ERROR :You are already authenticated\r\n");
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
    Client &client = getClientByFd(clientFd);
    if (!check_authentication(client, "JOIN")) return;

    if (client.getNickname().empty()) {
        sendMessage(clientFd, "ERROR :You must set your nickname first\r\n");
        return;
    }
    std::cout << "Client " << clientFd << " is attempting to join channel with params: " << params << std::endl;

    bool paramPassword = false;
    if (params.empty()) 
        return sendMessage(clientFd, "ERROR :No channel specified\r\n");

    if (params[0] != '#') 
        return sendMessage(clientFd, "ERROR :Invalid channel name\r\n");

    if (params.find(' ') != std::string::npos) {
        paramPassword = true;
    }
    std::string channelName;
    if (paramPassword) {
        channelName = params.substr(0, params.find(' '));
    } else {
        channelName = params;
    }

    for (size_t i = 0; i < channels.size(); ++i) {
        std::cout << "Checking existing channel: " << channels[i].getName() << std::endl;
        if (channels[i].getName() == channelName) {
            if (isPartOfChannel(clientFd, channelName)) {
                return sendMessage(clientFd, "ERROR :You are already a member of channel " + channelName + "\r\n");
            }
            if (channels[i].getUserLimit() > 0 && channels[i].getMembers().size() >= channels[i].getUserLimit()) {
                return sendMessage(clientFd, "471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n");
            }
            if (channels[i].getProtected()){
                std::cout << "Channel " << params << " is protected. Checking password." << std::endl;
                size_t spacePos = params.find(' ');
                std::string channelName = params.substr(0, spacePos);
                std::string password = params.substr(spacePos + 1); 
                if (password != channels[i].getPassword())
                    return sendMessage(clientFd, "ERROR :Cannot join protected channel\r\n");
            }
            if (channels[i].getInviteOnly() && !channels[i].isInvited(clientFd)) {
                std::cout << "Channel " << params << " is invite-only. Checking invitation." << std::endl;
                return sendMessage(clientFd, "ERROR :Cannot join invite-only channel without invitation\r\n");
            }
            channels[i].addMember(clientFd);
            std::cout << "Client " << clientFd << " joined existing channel: " << channelName << std::endl;
            sendMessage(clientFd, "Joined channel " + channelName + "\r\n");
            return;
        }
    }
    Channel newChannel(channelName);
    std::cout << "Creating new channel: " << channelName << std::endl;
    newChannel.addMember(clientFd);
    newChannel.addOperator(clientFd);
    channels.push_back(newChannel);
    sendMessage(clientFd, "Joined channel " + channelName + "\r\n");
    return;
}

// void Server::PRIVMSG(int clientFd, const std::string &params) {

//     if (params.empty()) {
//         sendMessage(clientFd, "ERROR :No target specified\r\n");
//         return;
//     }
//     size_t spacePos = params.find(' ');
//     if (spacePos == std::string::npos) {
//         sendMessage(clientFd, "ERROR :No message specified\r\n");
//         return;
//     }
//     std::string target = params.substr(0, spacePos);
//     // Target is a channel
//     if (target[0] == '#') {
//         std::string message = params.substr(spacePos + 1);
//         if (message.empty()) {
//             sendMessage(clientFd, "ERROR :No message specified\r\n");
//             return;
//         }
//         MSG_CHANNEL(clientFd, target, message);
//         return;
//     }
//     // Target is a user
//     std::string message = params.substr(spacePos + 1);
// 	if (message[0] == ':')
// 		message.erase(0, 1);
// 	int targetFd = 0;
//     for (size_t i = 0; i < clients.size(); ++i) {
//         if (clients[i].getNickname() == target) {
// 			targetFd = clients[i].getFd();
//             break;
//         }
//     }
//     if (targetFd) {
// 		std::cout << "Message sent to " + target << std::endl;
//         sendMessage(targetFd, message + "\r\n");
//     } else {
//         sendMessage(clientFd, "ERROR :No such nickname\r\n");
//     }
// }


// void Server::MSG_CHANNEL(int clientFd, const std::string channelName,const std::string message) {

//     std::cout << "Client " << clientFd << " is sending message to channel " << channelName << ": " << message << std::endl;
//     Client &client = getClientByFd(clientFd);
//     for (size_t i = 0; i < channels.size(); ++i) {
//         if (channels[i].getName() == channelName) {
//             if (isPartOfChannel(clientFd, channelName)) {
//                 const std::vector<int> &members = channels[i].getMembers();
//                 for (size_t j = 0; j < members.size(); ++j) {
//                     if (members[j] != clientFd) {
//                         sendMessage(members[j], "FROM " + client.getNickname() + " " + channelName + ": " + message + "\r\n");
//                     }
//                 }
//             } else {
//                 sendMessage(clientFd, "ERROR :You are not a member of channel " + channelName + "\r\n");
//             }
//             return;
//         }
//     }
//     sendMessage(clientFd, "ERROR :No such channel\r\n");
// }

void Server::PRIVMSG(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, "461 PRIVMSG :No recipient given\r\n");
        return;
    }
    size_t spacePos = params.find(' ');
    if (spacePos == std::string::npos) {
        sendMessage(clientFd, "412 PRIVMSG :No text to send\r\n");
        return;
    }
    std::string target = params.substr(0, spacePos);
    std::string message = params.substr(spacePos + 1);
    if (!message.empty() && message[0] == ':') {
        message = message.substr(1);
    }
    if (message.empty()) {
        sendMessage(clientFd, "412 PRIVMSG :No text to send\r\n");
        return;
    }
    Client &sender = getClientByFd(clientFd);
    // Target is a channel
    if (target[0] == '#') {
        MSG_CHANNEL(clientFd, target, message);
        return;
    }
    // Target is a user
    int targetFd = -1;
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getNickname() == target) {
            targetFd = clients[i].getFd();
            break;
        }
    }
    if (targetFd == -1) {
        sendMessage(clientFd, "401 " + target + " :No such nick/channel\r\n");
        return;
    }
    std::string formattedMessage = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";
    std::cout << "Sending PRIVMSG to " << target << ": " << formattedMessage << std::endl;
    sendMessage(targetFd, formattedMessage);
}

void Server::MSG_CHANNEL(int clientFd, const std::string channelName, const std::string message) {
    std::cout << "Client " << clientFd << " is sending message to channel " << channelName << ": " << message << std::endl;
    
    Client &sender = getClientByFd(clientFd);
    Channel *channel = NULL;
    
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            channel = &channels[i];
            break;
        }
    }
    if (!channel) {
        sendMessage(clientFd, "403 " + channelName + " :No such channel\r\n");
        return;
    }
    if (!channel->isMember(clientFd)) {
        sendMessage(clientFd, "404 " + channelName + " :Cannot send to channel\r\n");
        return;
    }
    std::string formattedMessage = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost PRIVMSG " + channelName + " :" + message + "\r\n";
    const std::vector<int> &members = channel->getMembers();
    for (size_t j = 0; j < members.size(); ++j) {
        if (members[j] != clientFd) {
            sendMessage(members[j], formattedMessage);
        }
    }
    std::cout << "Message broadcast to channel " << channelName << std::endl;
}

std::string Server::toString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
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

void Server::MODE(int clientFd, const std::string &params) {
    std::cout << "Client " << clientFd << " is attempting to change mode with params: " << params << std::endl;
    if (params.empty()) {
        sendMessage(clientFd, "ERROR :No channel specified\r\n");
        return;
    }
    // Parse parameters
    std::vector<std::string> tokens = split_string(params, ' ');
    if (tokens.size() < 1) {
        sendMessage(clientFd, "461 MODE :Not enough parameters\r\n");
        return;
    }
    std::string channelName = tokens[0];
    if (channelName[0] != '#') {
        sendMessage(clientFd, "476 " + channelName + " :Invalid channel name\r\n");
        return;
    }
    // Find the channel
    Channel *channel = NULL;
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            channel = &channels[i];
            break;
        }
    }
    if (!channel) {
        sendMessage(clientFd, "403 " + channelName + " :No such channel\r\n");
        return;
    }
    // If no mode specified, return current modes
    if (tokens.size() == 1) {
        std::string modeString = "+";
        if (channel->getInviteOnly()) modeString += "i";
        if (channel->getTopicRestricted()) modeString += "t";
        if (!channel->getPassword().empty()) modeString += "k";
        if (channel->getUserLimit() > 0) modeString += "l";
        
        std::string response = "324 " + channelName + " " + modeString;
        if (!channel->getPassword().empty()) {
            response += " " + channel->getPassword();
        }
        if (channel->getUserLimit() > 0) {
            response += " " + toString(channel->getUserLimit());
        }
        response += "\r\n";
        sendMessage(clientFd, response);
        return;
    }
    // Check if user is in channel
    if (!channel->isMember(clientFd)) {
        sendMessage(clientFd, "442 " + channelName + " :You're not on that channel\r\n");
        return;
    }
    // Check if user is channel operator
    if (!channel->isOperator(clientFd)) {
        sendMessage(clientFd, "482 " + channelName + " :You're not channel operator\r\n");
        return;
    }
    std::string modes = tokens[1];
    if (modes.empty() || (modes[0] != '+' && modes[0] != '-')) {
        sendMessage(clientFd, "472 " + modes + " :Unknown mode char\r\n");
        return;
    }
    char sign = modes[0];
    size_t paramIndex = 2; // Start from token index 2 for parameters
    // Process each mode character
    for (size_t i = 1; i < modes.length(); ++i) {
        char modeChar = modes[i];
        if (modeChar == 'i') { // Invite only
            if (sign == '+') {
                channel->setInviteOnly(true);
                sendMessage(clientFd, "MODE " + channelName + " +i\r\n");
            } else {
                channel->setInviteOnly(false);
                sendMessage(clientFd, "MODE " + channelName + " -i\r\n");
            }
        }
        else if (modeChar == 't') { // Topic restriction
            if (sign == '+') {
                channel->toggleTopic(true);
                sendMessage(clientFd, "MODE " + channelName + " +t\r\n");
            } else {
                channel->toggleTopic(false);
                sendMessage(clientFd, "MODE " + channelName + " -t\r\n");
            }
        }
        else if (modeChar == 'k') { // Channel key/password
            if (sign == '+') {
                if (paramIndex >= tokens.size()) {
                    sendMessage(clientFd, "461 MODE :Key (password) required for +k mode\r\n");
                    continue;
                }
                std::string password = tokens[paramIndex++];
                if (!channel->getPassword().empty()) {
                    sendMessage(clientFd, "467 " + channelName + " :Channel key already set\r\n");
                    continue;
                }
                channel->setPassword(password);
                channel->setProtected(true);
                sendMessage(clientFd, "MODE " + channelName + " +k " + password + "\r\n");
            } else {
                channel->removePassword();
                sendMessage(clientFd, "MODE " + channelName + " -k\r\n");
            }
        }
        else if (modeChar == 'o') { // Give/take operator privilege
            if (paramIndex >= tokens.size()) {
                sendMessage(clientFd, "461 MODE :Nickname required for +o mode\r\n");
                continue;
            }
            std::string targetNick = tokens[paramIndex++];
            int targetFd = getClientFdByNickname(targetNick);
            
            if (targetFd == -1) {
                sendMessage(clientFd, "401 " + targetNick + " :No such nick\r\n");
                continue;
            }
            
            if (!channel->isMember(targetFd)) {
                sendMessage(clientFd, "441 " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
                continue;
            }
            
            if (sign == '+') {
                if (!channel->isOperator(targetFd)) {
                    channel->addOperator(targetFd);
                    sendMessage(clientFd, "MODE " + channelName + " +o " + targetNick + "\r\n");
                    // Notify the user they are now operator
                    sendMessage(targetFd, "You are now operator of " + channelName + "\r\n");
                }
            } else {
                if (channel->isOperator(targetFd)) {
                    channel->removeOperator(targetFd);
                    sendMessage(clientFd, "MODE " + channelName + " -o " + targetNick + "\r\n");
                    // Notify the user they are no longer operator
                    sendMessage(targetFd, "You are no longer operator of " + channelName + "\r\n");
                }
            }
        }
        else if (modeChar == 'l') { // User limit
            if (sign == '+') {
                if (paramIndex >= tokens.size()) {
                    sendMessage(clientFd, "461 MODE :Limit value required for +l mode\r\n");
                    continue;
                }
                std::string limitStr = tokens[paramIndex++];
                int limit = std::atoi(limitStr.c_str());
                if (limit <= 0) {
                    sendMessage(clientFd, "999 " + channelName + " :Invalid limit value\r\n");
                    continue;
                }
                channel->setUserLimit(limit);
                sendMessage(clientFd, "MODE " + channelName + " +l " + limitStr + "\r\n");
            } else {
                channel->setUserLimit(0);
                sendMessage(clientFd, "MODE " + channelName + " -l\r\n");
            }
        }
        else {
            sendMessage(clientFd, "472 " + std::string(1, modeChar) + " :Unknown mode char\r\n");
        }
    }
    // Broadcast mode changes to all channel members
    std::string modeChangeMsg = ":" + getClientByFd(clientFd).getNickname() + " MODE " + channelName + " " + modes;
    for (size_t i = 2; i < tokens.size() && i < paramIndex; ++i) {
        modeChangeMsg += " " + tokens[i];
    }
    modeChangeMsg += "\r\n";
    
    const std::vector<int> &members = channel->getMembers();
    for (size_t i = 0; i < members.size(); ++i) {
        if (members[i] != clientFd) {
            sendMessage(members[i], modeChangeMsg);
        }
    }
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
        if (!topic.empty() && topic[0] == ':') {
        topic = topic.substr(1); // supprime le premier caractère ':'
    }
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
	std::cout << "Client " << clientFd << " is attempting to register USER with params: " << params << std::endl;

	Client &client = getClientByFd(clientFd);

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
		sendMessage(clientFd, msg + "\r\n");
		return;
	}	
	client.setUsername(args[0]);
	client.setRealname(realname);
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
            std::cout << "Client " << clientFd << " parted from existing channel: " << params << std::endl;
            sendMessage(clientFd, "Parted from channel " + params + "\r\n");
            return;
        }
    }
    sendMessage(clientFd, "ERROR :No such channel\r\n");
    return;
}

void Server::KICK(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, "ERROR :No parameters specified for KICK\r\n");
        return;
    }
    Client &client = getClientByFd(clientFd);
    std::string channelName;
    std::vector<std::string> tokens = split_string(params, ' ');
    if (tokens.size() < 2) {
        sendMessage(clientFd, "ERROR :Not enough parameters for KICK\r\n");
        return;
    }
    channelName = tokens[0];
    size_t i;
    for (i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName)
            break;
    }
    std::string nickToKick = tokens[1];
    if (!channels[i].isMember(clientFd)) {
        sendMessage(clientFd, "ERROR :You are not a member of the channel\r\n");
        return;
    }
    if (!channels[i].isOperator(clientFd)) {
        sendMessage(clientFd, "ERROR :You are not an operator of the channel\r\n");
        return;
    }
    for (size_t j = 0; j < clients.size(); ++j) {
        if (clients[j].getNickname() == nickToKick) {
            int kickFd = clients[j].getFd();
            if (!channels[i].isMember(kickFd)) {
                sendMessage(clientFd, "ERROR :User is not a member of the channel\r\n");
                return;
            }
            channels[i].removeMember(kickFd);
            sendMessage(kickFd, "You have been kicked from " + channelName + " by " + client.getNickname() + "\r\n");
            sendMessage(clientFd, nickToKick + " has been kicked from " + channelName + "\r\n");
            return;
        }
    }
}

void Server::INVITE(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, "ERROR :No parameters specified for INVITE\r\n");
        return;
    }
    Client &client = getClientByFd(clientFd);
    std::vector<std::string> tokens = split_string(params, ' ');
    if (tokens.size() < 2) {
        sendMessage(clientFd, "ERROR :Not enough parameters for INVITE\r\n");
        return;
    }
    std::string nickToInvite = tokens[0];
    std::string channelName = tokens[1];
    std::cout << "INVITE: Client " << clientFd << " is inviting " << nickToInvite << " to channel " << channelName << std::endl;
    size_t i;
    for (i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName)
            break;
    }
    if (i >= channels.size()) {
        sendMessage(clientFd, "ERROR :No such channel\r\n");
        return;
    }
    std::cout << "INVITE: Looking for channel " << channels[i].getName() << std::endl;
    if (isPartOfChannel(clientFd, channelName) == false) {
        sendMessage(clientFd, "ERROR :You are not a member of the channel\r\n");
        return;
    }
    for (size_t j = 0; j < clients.size(); ++j) {
        if (clients[j].getNickname() == nickToInvite) {
            int inviteFd = clients[j].getFd();
            if (channels[i].isMember(inviteFd)) {
                sendMessage(clientFd, "ERROR :User is already a member of the channel\r\n");
                return;
            }
            channels[i].addInvitedMember(inviteFd);
            sendMessage(inviteFd, "You have been invited to " + channelName + " by " + client.getNickname() + "\r\n");
            sendMessage(clientFd, nickToInvite + " has been invited to " + channelName + "\r\n");
            return;
        }
    }
    sendMessage(clientFd, "ERROR :No such nickname\r\n");
}

void Server::PING(int clientFd, const std::string &params) {
    if (params.empty()) {
        sendMessage(clientFd, "409 :No origin specified\r\n");
        return;
    }
    std::string response = "PONG";
    if (!params.empty()) {
        response += " " + params;
    }
    response += "\r\n";
    sendMessage(clientFd, response);
}

