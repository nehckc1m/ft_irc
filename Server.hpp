#pragma once

#include "Client.hpp"
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
#include "Channel.hpp"



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

        //COMMAND HANDLERS
        void PASS(int clientFd, const std::string &params);
        void JOIN(int clientFd, const std::string &params);
};