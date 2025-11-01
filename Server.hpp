#pragma once

#include "Client.hpp"
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
        static bool _signal;
        int getPort() const;

        
    public:
        Server(int port, const std::string &password);
        ~Server();

        //std::string getAddress() const;
        //int getPort() const;
        void createSocket();
        int initSocket();
        void bindAndListen(int socket);
        void addSocketToPoll(int socket);
        void run();
        void closeFds();
        void acceptNewConnection();
        static void SignalHandler(int signum);
        void removeClient(int clientFd);
};