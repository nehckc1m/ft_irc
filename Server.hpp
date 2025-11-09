#pragma once

#include <map>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "Reply.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#define RST  "\033[0m"
#define GRAY "\033[90m"
#define P_CYAN "\033[38;5;123m"

#ifndef DEBUG
# define DEBUG 0
#endif

// Returns vector of strings created by splitting the string by separator sep
std::vector<std::string> split_string(const std::string s, char sep);

class Server {
    private:
        int _port;
        std::string _password;
        int _servSocket;
        std::vector<struct pollfd> poll_fds;
        std::vector<Client> clients; // client tbd later
        std::vector<Channel> channels; // channel tbd later
        std::map<int, std::string> clientBuffers; // Buffer for each client
        static bool _signal;
        int getPort() const;
        void sendMessage(int clientFd, const std::string &message);
        Client &getClientByFd(int clientFd);

        
    public:
        Server(int port, const std::string &password);
        ~Server();

        //std::string getAddress() const;
        //int getPort() const;
        void handleCommand(int clientFd, const std::string &command);
        void processBuffer(int clientFd, char *buffer, int bytes);
        void createSocket();
        int initSocket();
        void bindAndListen(int socket);
        void addSocketToPoll(int socket);
        void run();
        void closeFds();
        void acceptNewConnection();
        static void SignalHandler(int signum);
        void removeClient(int clientFd);
		bool nicknameExists(const std::string &nickname) const;
		int getClientFdByNickname(const std::string &nickname) const;


        //COMMAND HANDLERS
        void PASS(int clientFd, const std::string &params);
        void JOIN(int clientFd, const std::string &params);
        void PRIVMSG(int clientFd, const std::string &params);
        void MSG_CHANNEL(int clientFd, const std::string channelName, const std::string message);
        void MODE(int clientFd, const std::string &params);
        void TOPIC(int clientFd, const std::string &params);
		void NICK(int clientFd, const std::string &params);
		void USER(int clientFd, const std::string &params);
		void PART(int clientFd, const std::string &params);
        void KICK(int clientFd, const std::string &params);
        void INVITE(int clientFd, const std::string &params);
		void PING(int clientFd, const std::string &params);
		void CAP(int clientFd, const std::string &params);
		void user_mode(int clientFd, const std::string &params);

        //UTILITY FUNCTIONS
        bool isPartOfChannel(int clientFd, const std::string &channelName);
        int check_authentication(Client &client, std::string cmd);
        int getFdByNickname(const std::string &nickname);
        void flushSendBuffer(Client &client);
        void enablePollOut(int clientFd);
        void disablePollOut(int clientFd);
};