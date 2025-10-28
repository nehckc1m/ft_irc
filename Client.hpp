#pragma once

#include <string>
#include <vector>

class Client {
    public:
        Client(int socketFd);
        ~Client();

        int getFd() const;
        std::string getAddress() const;
    private:
        int _fd;
        std::string _nickname;
        std::string _username;
        std::vector<std::string> _channels;
        bool _isAuthenticated;
        

};
