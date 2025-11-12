#pragma once

#include <map>
#include <sstream>
#include "Client.hpp"

#define RST		"\033[0m"
#define RED		"\033[31m"
#define GREEN	"\033[32m"
#define BLUE	"\033[34m"
#define MAGENTA "\033[35m"
#define CYAN	"\033[36m"
#define GRAY	"\033[90m"
#define YELLOW	"\033[38;5;214m"
#define ORANGE	"\033[38;5;215m"

enum e_reply {
	// Success replies
	RPL_WELCOME	= 1,				// 001 USER
	RPL_YOURHOST = 2,				// 002 USER
	RPL_CREATED = 3,				// 003 USER
	RPL_MYINFO = 4,					// 004 USER
	RPL_CHANNELMODEIS = 324,		// 324 MODE
	RPL_NOTOPIC = 331,				// 331 TOPIC
	RPL_TOPIC = 332,				// 332 TOPIC
	RPL_YOUREOPER = 381,			// 381 USER

	// Error replies
	ERR_NOSUCHNICK = 401,			// 401 PRIVMSG, INVITE, KICK
	ERR_NOSUCHCHANNEL = 403,		// 403 JOIN, TOPIC, MODE, PRIVMSG
	ERR_CANNOTSENDTOCHAN = 404,		// 404 PRIVMSG
	ERR_NOTEXTTOSEND = 411,			// 411 PRIVMSG
	ERR_UNKNOWNCOMMAND = 421,		// 421 unknown command
	ERR_NONICKNAMEGIVEN = 431,		// 431 NICK
	ERR_ERRONEUSNICKNAME = 432,		// 432 NICK
	ERR_NICKNAMEINUSE = 433,		// 433 NICK
	ERR_NOTONCHANNEL = 442,			// 442 MODE, TOPIC, KICK, PRIVMSG
	ERR_USERONCHANNEL = 443,		// 443 INVITE, JOIN
	ERR_NOTREGISTERED = 451,		// 451 USER (no NICK)
	ERR_NEEDMOREPARAMS = 461,		// 461 commands
	ERR_ALREADYREGISTRED = 462,		// 462 PASS, USER
	ERR_PASSWDMISMATCH = 464,		// 464 PASS
	ERR_CHANNELISFULL = 471,		// 471 JOIN
	ERR_CHANNELPASSWORDMISMATCH = 475,	// 475 JOIN
	ERR_INVITEONLYCHAN = 473,		// 473 JOIN
	ERR_UNKNOWNMODE = 472,			// 472 MODE
	ERR_CHANOPRIVSNEEDED = 482,		// 482 MODE, TOPIC, KICK
	ERR_NOOPERHOST = 491,			// 491 OPER
	ERR_USERSDONTMATCH = 502,		// 502 MODE
};

class Reply {
	std::map<int, std::string> message;
	std::string cmd;
	Client& client;

public:
	Reply(const std::string &command, Client &c);
	void init();
	std::string msg(e_reply code);
	std::string msg(e_reply code, const std::string &param1);
	std::string msg(e_reply code, const std::string &param1, const std::string &param2);
	std::string msg(e_reply code, const std::string &param1, const std::string &param2, const std::string &param3);
};
