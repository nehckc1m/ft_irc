#pragma once

#include <string>
#include <vector>

class Client {
    public:
        Client(int socketFd);
        ~Client();

        int getFd() const;
        std::string getNickname() const ;
        std::string getIpAddress() const ;
        std::string getUser() const ;
        void setNickname(const std::string &nickname);
        void setAuthenticate();
        bool isAuthenticated() const ;

    private:
        int _fd;
        std::string _nickname;
        std::string _username;
        std::string _ipAddress;
        std::vector<std::string> _channels;
        bool _authenticated;
};
