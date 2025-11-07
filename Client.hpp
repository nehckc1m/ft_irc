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
		std::string getUsername() const ;
		std::string getRealname() const ;
		std::string getHostname() const ;
		std::string getServername() const ;
        void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
        void setAuthenticate();
        bool isAuthenticated() const ;
       /* void queueMessage(const std::string &message);
        std::string &getSendBuffer();
        bool hasPendingMessages() const ;*/ //TBD

    private:
        int _fd;
        std::string _nickname;
        std::string _username;
        std::vector<std::string> _channels;
        std::string _sendBuffer;
        bool _authenticated;
};
