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
		void setRealname(const std::string &realname);
		void setHostname(const std::string &hostname);
		void setServername(const std::string &servername);
        void setAuthenticate();
        bool isAuthenticated() const ;

    private:
        int _fd;
        std::string _nickname;
        std::string _username;
		std::string _realname;
		std::string _hostname;
		std::string _servername;
        std::string _ipAddress;
        std::vector<std::string> _channels;
        bool _authenticated;
};
