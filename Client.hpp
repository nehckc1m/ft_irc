#pragma once

#include <string>
#include <vector>

class Client {
    public:
        Client(int socketFd);
        //Client &operator=(const Client &other);
        ~Client();

        int getFd() const;
        std::string getNickname() const ;
        std::string getIpAddress() const ;
		std::string getUsername() const ;
		std::string getRealname() const ;
        void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);
        void setAuthenticate();
        bool isAuthenticated() const ;
        // Message queueing methods
        void queueMessage(const std::string &message);
        std::string &getSendBuffer();
        bool hasPendingMessages() const ;

    private:
        int _fd;
        std::string _nickname;
        std::string _username;
		std::string _realname;
        std::string _ipAddress;
        std::vector<std::string> _channels;
        std::string _sendBuffer;
        bool _authenticated;
};
