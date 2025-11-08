#pragma once

#include <map>
#include <sstream>
#include "Client.hpp"

enum e_reply {
	// Errors
	RPL_WELCOME	= 001,			// NICK
	RPL_YOUREOPER = 381,		// USER
	// Errors

	ERR_UNKNOWNCOMMAND = 421,
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
	std::string cmd;
	Client& client;

public:
	Reply(const std::string &command, Client &c);
	void init();
	std::string msg(e_reply code);
};
