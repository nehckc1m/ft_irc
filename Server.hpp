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

class Server {
    private:
        int _port;
        std::string _password;
        bool signal;
        int _servSocket;
        std::vector<struct pollfd> poll_fds;
        std::vector<Client> clients; // client tbd later
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
        void SignalHandler(int signum);
};