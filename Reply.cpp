#include "Reply.hpp"

Reply::Reply(const std::string &cmd, Client& c) : cmd(cmd), client(c) { init(); }

std::string Reply::msg(e_reply code)
{
	return message[code];
}

std::string Reply::msg(e_reply code, const std::string &param1){
	std::string result = message[code];
	size_t pos = result.find("$1");
	if (pos != std::string::npos) {
		result.replace(pos, 2, param1);
	}
	return result;
}

std::string Reply::msg(e_reply code, const std::string &param1, const std::string &param2){
	std::string result = message[code];
	size_t pos = result.find("$1");
	if (pos != std::string::npos) {
		result.replace(pos, 2, param1);
	}
	pos = result.find("$2");
	if (pos != std::string::npos) {
		result.replace(pos, 2, param2);
	}
	return result;
}

std::string Reply::msg(e_reply code, const std::string &param1, const std::string &param2, const std::string &param3){
	std::string result = message[code];
	size_t pos = result.find("$1");
	if (pos != std::string::npos) {
		result.replace(pos, 2, param1);
	}
	pos = result.find("$2");
	if (pos != std::string::npos) {
		result.replace(pos, 2, param2);
	}
	pos = result.find("$3");
	if (pos != std::string::npos) {
		result.replace(pos, 2, param3);
	}
	return result;
}

std::string str(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

void Reply::init()
{
	const std::string srv = ":localhost ";
	const std::string nick = " " + (!client.getNickname().empty() ? client.getNickname() : "*") + " ";
	message[RPL_WELCOME] = srv + "001" + nick + ":Welcome to the FT_IRC server!\r\n";
	message[RPL_YOURHOST] = srv + "002" + nick + ":Your host is localhost, running version 1.0\r\n";
	message[RPL_CREATED] = srv + "003" + nick + ":This server was created today\r\n";
	message[RPL_MYINFO] = srv + "004" + nick + " :ft_irc 42 Le Havre\r\n";
	message[RPL_CHANNELMODEIS] = srv + str(RPL_CHANNELMODEIS) + nick + "$1 $2\r\n"; 
	message[RPL_NOTOPIC] = srv + str(RPL_NOTOPIC) + nick + "$1 :No topic is set\r\n";
	message[RPL_TOPIC] = srv + str(RPL_TOPIC) + nick + "$1 :$2\r\n";
	message[RPL_YOUREOPER] = srv + str(RPL_YOUREOPER) + nick + ":You are now an IRC operator\r\n";
	message[ERR_NOSUCHNICK] = srv + str(ERR_NOSUCHNICK) + nick + "$1" + " :No such nick/channel\r\n";
	message[ERR_CANNOTSENDTOCHAN] = srv + str(ERR_CANNOTSENDTOCHAN) + nick + cmd + " :Cannot send to channel\r\n";
	message[ERR_NONICKNAMEGIVEN] = srv + str(ERR_NONICKNAMEGIVEN) + " :No nickname given\r\n";
	message[ERR_ERRONEUSNICKNAME] = srv + str(ERR_ERRONEUSNICKNAME) + nick + ":Erroneus nickname\r\n";
	message[ERR_NICKNAMEINUSE] = srv + str(ERR_NICKNAMEINUSE) + nick + ":Nickname is already in use\r\n";
	message[ERR_NEEDMOREPARAMS] = srv + str(ERR_NEEDMOREPARAMS) + cmd + " :Not enough parameters\r\n";
	message[ERR_ALREADYREGISTRED] = srv + str(ERR_ALREADYREGISTRED) + nick + ":You may not reregister\r\n";
	message[ERR_UNKNOWNCOMMAND] = srv + str(ERR_UNKNOWNCOMMAND) + nick + cmd + " :Unknown command\r\n";
	message[ERR_USERSDONTMATCH] = srv + str(ERR_USERSDONTMATCH) + " " + "$1" + " :Cannot change mode for other users\r\n";
	message[ERR_NOTREGISTERED] = srv + str(ERR_NOTREGISTERED) + " " + cmd + " :You have not registered\r\n";
	message[ERR_NOSUCHCHANNEL] = srv + str(ERR_NOSUCHCHANNEL) + nick + "$1" + " :No such channel\r\n";
	message[ERR_NOTONCHANNEL] = srv + str(ERR_NOTONCHANNEL) + nick + "$1" + " :You're not on that channel\r\n";
	message[ERR_USERONCHANNEL] = srv + str(ERR_USERONCHANNEL) + nick + "$1" + " :is already on channel\r\n";
	message[ERR_PASSWDMISMATCH] = srv + str(ERR_PASSWDMISMATCH) + nick + ":Password incorrect\r\n";
	message[ERR_CHANNELISFULL] = srv + str(ERR_CHANNELISFULL) + nick + "$1" + " :Cannot join channel (+l)\r\n";
	message[ERR_CHANNELPASSWORDMISMATCH] = srv + str(ERR_CHANNELPASSWORDMISMATCH) + nick + "$1" + " :Cannot join channel (+k)\r\n";
	message[ERR_INVITEONLYCHAN] = srv + str(ERR_INVITEONLYCHAN) + nick + "$1" + " :Cannot join channel (+i)\r\n";
	message[ERR_UNKNOWNMODE] = srv + str(ERR_UNKNOWNMODE) + nick + cmd + " :is unknown mode char to me\r\n";
	message[ERR_CHANOPRIVSNEEDED] = srv + str(ERR_CHANOPRIVSNEEDED) + nick + "$1" + " :You're not channel operator\r\n";
	message[ERR_NOOPERHOST] = srv + str(ERR_NOOPERHOST) + nick + ":No O-lines for your host\r\n";
	message[ERR_NOTEXTTOSEND] = srv + str(ERR_NOTEXTTOSEND) + ":No text to send\r\n"; 
}

