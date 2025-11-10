#include "Server.hpp"
#include "Reply.hpp"
#include <cstdlib>

void Server::PASS(int clientFd, const std::string &params) {
    
    Client &client = getClientByFd(clientFd);
    if (client.isAuthenticated()) {
        return sendMessage(clientFd, ":localhost PASS :You are already authenticated\r\n");
    }
	if (params == _password) {
        client.setAuthenticate();
        std::cout << "Client " << clientFd << " authenticated successfully." << std::endl;
    }
    else {
        std::cout << "Client " << clientFd << " provided incorrect password." << std::endl;
        sendMessage(clientFd, ":localhost PASS :Password incorrect\r\n");
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
        channels.push_back(Channel(channelName));
        channels.back().addMember(clientFd);
        channels.back().addOperator(clientFd);
        channel = &channels.back();
    } else {
        if (isPartOfChannel(clientFd, channelName)) {
            sendMessage(clientFd, ":localhost 443 " + client.getNickname() + " " + channelName + " :You're already on that channel\r\n");
            return;
        }
        channel->addMember(clientFd);
    }
	std::vector<std::string> channelModes;
	if (channel->getInviteOnly())
		channelModes.push_back("i");
	if (channel->getTopicRestricted())
		channelModes.push_back("t");
	if (channel->getModerated())
		channelModes.push_back("m");
	if (channel->getLimit() > 0)
		channelModes.push_back("l");
	if (!channel->getPassword().empty())
		channelModes.push_back("k");
	std::string modes;
	if (!channelModes.empty()) {
		modes = "+";
		for (size_t i = 0; i < channelModes.size(); ++i) {
			modes += channelModes[i];;
		}
	}
	sendMessage(clientFd, ":localhost 321 " + client.getNickname() + " " + channelName + " " + modes + "\r\n");
    sendMessage(clientFd, ":" + client.getNickname() + " JOIN " + channelName + "\r\n");
    if (!channel->getTopic().empty()) {
        sendMessage(clientFd, ":localhost 332 " + client.getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
    }
    std::string names;

    for (unsigned long fd = 0; fd < channel->getMembers().size(); ++fd) {
		if(channel->isOperator(channel->getMembers()[fd])) 
			names += "@";
        names += getClientByFd(channel->getMembers()[fd]).getNickname() + " ";
		
	}
    sendMessage(clientFd,
        ":localhost 353 " + client.getNickname() + " = " + channelName + " :" + names + "\r\n"
    );
    sendMessage(clientFd,
        ":localhost 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list.\r\n"
    );
}


void Server::PRIVMSG(int clientFd, const std::string &params) {

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
	if (message.empty()) {
		sendMessage(clientFd, "ERROR :No message specified\r\n");
		return;
	}
	if (message[0] != ':')
		message = ":" + message;
	int targetFd = -1;
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getNickname() == target) {
			targetFd = clients[i].getFd();
            break;
        }
    }
	if (targetFd == -1) {
		sendMessage(clientFd, "ERROR :No such nickname\r\n");
		return;
	}
    if (targetFd != -1) {
		std::cout << "Message sent to " + target << std::endl;
        sendMessage(targetFd, ":" + getClientByFd(clientFd).getNickname() + " PRIVMSG " + target + " " + message + "\r\n");
    } else {
        sendMessage(clientFd, "ERROR :No such nickname\r\n");
    }
}

void Server::MSG_CHANNEL(int clientFd, const std::string channelName,const std::string message) {

    std::cout << "Client " << clientFd << " is sending message to channel " << channelName << ": " << message << std::endl;
    Client &client = getClientByFd(clientFd);
    for (size_t i = 0; i < channels.size(); ++i) {
        if (channels[i].getName() == channelName) {
            if (isPartOfChannel(clientFd, channelName)) {
                const std::vector<int> &members = channels[i].getMembers();
				if (message.empty()) {
					sendMessage(clientFd, "ERROR :No message specified\r\n");
					return;
				}
                for (size_t j = 0; j < members.size(); ++j) {
                    if (members[j] != clientFd) {
						sendMessage(members[j], ":" + client.getNickname() + " PRIVMSG " + channelName + " :" + message + "\r\n");
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

void Server::user_mode(int clientFd, const std::string &params) {
	Reply reply("MODE", getClientByFd(clientFd));
	std::vector<std::string> args = split_string(params, ' ');
	if (args.size() != 2) {
		sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
		return;
	}
	if (getClientByFd(clientFd).getNickname() != args[0]) {
		sendMessage(clientFd, reply.msg(ERR_USERSDONTMATCH));
		return;
	}
}

void Server::MODE(int clientFd, const std::string &params) {

	Reply reply("MODE", getClientByFd(clientFd));
	if (params.empty()) {
		sendMessage(clientFd, reply.msg(ERR_NEEDMOREPARAMS));
        return;
    }
	// User mode not supported and added to remove errors in irssi client when send MODE <nickname> +i
    if (params[0] != '#') {
        user_mode(clientFd, params);
        return;
    }
	// Channel mode
	std::string channelName = params;
	std::string modeChanges;
	size_t spacePos = params.find(' ');
	if (spacePos != std::string::npos) {
		channelName = params.substr(0, spacePos);
		modeChanges = params.substr(spacePos + 1);
	}

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i].getName() == channelName) {
			if (!isPartOfChannel(clientFd, channelName)) {
				sendMessage(clientFd, "ERROR :You are not a member of channel " + channelName + "\r\n");
				return;
			}
			// For simplicity, we just acknowledge the mode change without actually changing anything
			if (!modeChanges.empty()) {
				if (!channels[i].isOperator(clientFd)) {
					sendMessage(clientFd, "ERROR :You are not a channel operator\r\n");
					return;
				}
			
				if (modeChanges[0] == '+') {
					if (modeChanges[1] == 't') {
						channels[i].toggleTopic();
					}
					if (modeChanges[1] == 'o') {
						std::string targetNick = modeChanges.substr(3); // Assuming format is +o <nick>
						if (targetNick.empty()) {
							sendMessage(clientFd, "ERROR :No nickname specified for operator mode\r\n");
							return;
						}
						
						int targetFd = getClientFdByNickname(targetNick);
						channels[i].addOperator(targetFd);
					}
					if (modeChanges[1] == 'i') {
						channels[i].setInviteOnly(true);
					}
					if (modeChanges[1] == 'k') {
						std::string password = modeChanges.substr(3); // Assuming format is +k <password>
						channels[i].setPassword(password);
					}
					if (modeChanges[1] == 'l') {
						size_t limitPos = modeChanges.find(' ', 2); // Assuming format is +l <limit>
						if (limitPos != std::string::npos) {
							std::string limitValue = modeChanges.substr(limitPos + 1);
							int limit = std::atoi(limitValue.c_str());
							if (limit > 0) {
								channels[i].setUserLimit(limit);
							} else {
								sendMessage(clientFd, "ERROR :Invalid user limit\r\n");
							}
						}
					}

				} else if (modeChanges[0] == '-') {
					if (modeChanges[1] == 't') {
						channels[i].toggleTopic();
					}
					if (modeChanges[1] == 'o') {
						channels[i].removeOperator(clientFd);
					}
					if (modeChanges[1] == 'i') {
						channels[i].setInviteOnly(false);
					}
					if (modeChanges[1] == 'k') {
						channels[i].removePassword();
					}
					if (modeChanges[1] == 'l') {
						channels[i].setUserLimit(0); // Remove user limit
					}
				}

				sendMessage(clientFd, "Mode for " + channelName + " changed: " + modeChanges + "\r\n");
			} else {
				sendMessage(clientFd, "Current mode for " + channelName + ": [modes not implemented]\r\n");
			}
			return;
		}
	}
	sendMessage(clientFd, "ERROR :No such channel\r\n");
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

    Client &client = getClientByFd(clientFd);
    if (!client.isAuthenticated()) {
        return sendMessage(clientFd, ":localhost NICK :Password required\r\n");
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
        return sendMessage(clientFd, ":localhost USER :Nickname required\r\n");
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
		sendMessage(clientFd, msg + "\r\n");
		return;
	}	
	client.setUsername(args[0]);
	client.setRealname(realname);
	client.setRegistered();
	sendMessage(clientFd, reply.msg(RPL_WELCOME));
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
            sendMessage(clientFd, ":" + getClientByFd(clientFd).getNickname() + " PART " + channels[i].getName() + "\r\n");
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
    std::string nickToInvite = tokens[1];
    std::string channelName = tokens[0];
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
	sendMessage(clientFd, ":localhost PONG localhost :" + params + "\r\n");
}

void Server::CAP(int clientFd, const std::string &params) {
	(void) clientFd;
	(void) params;
}

