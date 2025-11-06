#pragma once

#include <map>
#include <sstream>

enum e_reply {
	// Errors
	RPL_WELCOME	= 001,			// NICK
	RPL_YOUREOPER = 381,		// USER
	// Errors

	ERR_NONICKNAMEGIVEN = 431,  // NICK 
	ERR_ERRONEUSNICKNAME = 432, // NICK
	ERR_NICKNAMEINUSE =    433,	// NICK

	ERR_NEEDMOREPARAMS	= 461,	// USER
	ERR_ALREADYREGISTRED = 462,
	ERR_PASSWDMISMATCH	= 464,	// PASS
	ERR_NOOPERHOST		= 491,

};

class Reply {
	std::map<int, std::string> message;
	std::string client;
	std::string cmd;
public:
	Reply(const std::string &command);
	void init();
	std::string msg(e_reply code);
};

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
	// NICK reply <nick>!<user>@<host>
	message[RPL_WELCOME] = str(RPL_WELCOME) + " " + cmd + " Welcome to the IRC server!\r\n";
	message[ERR_NONICKNAMEGIVEN] = str(ERR_NONICKNAMEGIVEN) + " :No nickname given\r\n";
	message[ERR_ERRONEUSNICKNAME] = str(ERR_ERRONEUSNICKNAME) + " <nick> :Erroneus nickname\r\n";
	message[ERR_NICKNAMEINUSE] = str(ERR_NICKNAMEINUSE) + " <nick> :Nickname is already in use\r\n";

	message[ERR_NEEDMOREPARAMS] = str(ERR_NEEDMOREPARAMS) + " " + cmd + " :Not enough parameters\r\n";
	message[ERR_ALREADYREGISTRED] = str(ERR_ALREADYREGISTRED) + " :Unauthorized command (already registered)\r\n";
}

Reply::Reply(const std::string &cmd) : cmd(cmd) { init(); }
