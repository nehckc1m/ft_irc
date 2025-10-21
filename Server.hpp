#pragma once

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>


class Server {
    private:
        std::string _password;
        int _port;

        
    public:
        Server(const std::string& address, int port);
        ~Server();

        //std::string getAddress() const;
        //int getPort() const;
        void initSocket();
};