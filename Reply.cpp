#include "Reply.hpp"

Reply::Reply(const std::string &cmd, Client& c) : cmd(cmd), client(c) { init(); }

std::string Reply::msg(e_reply code)
{
	return message[code];
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

	message[RPL_WELCOME] = srv + "001" + nick + ":Welcome to the IRC server!\r\n";
	message[ERR_NONICKNAMEGIVEN] = srv + str(ERR_NONICKNAMEGIVEN) + " :No nickname given\r\n";
	message[ERR_ERRONEUSNICKNAME] = srv + str(ERR_ERRONEUSNICKNAME) + nick + ":Erroneus nickname\r\n";
	message[ERR_NICKNAMEINUSE] = srv + str(ERR_NICKNAMEINUSE) + nick + ":Nickname is already in use\r\n";
	message[ERR_NEEDMOREPARAMS] = srv + str(ERR_NEEDMOREPARAMS) + nick + cmd + " :Not enough parameters\r\n";
	message[ERR_ALREADYREGISTRED] = srv + str(ERR_ALREADYREGISTRED) + nick + ":You may not reregister\r\n";
	message[ERR_UNKNOWNCOMMAND] = srv + str(ERR_UNKNOWNCOMMAND) + " " + cmd + " :Unknown command\r\n";
}

