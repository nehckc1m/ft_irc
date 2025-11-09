#pragma once


#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

class Channel {
    public:
        Channel(const std::string &name);
        Channel &operator=(const Channel &other);
        ~Channel();

        void addMember(int clientFd);
        void addOperator(int clientFd);
        void addInvitedMember(int clientFd);
        bool isInvited(int clientFd) const ;
        void removeMember(int clientFd);
        bool isMember(int clientFd) const;
        bool isOperator(int clientFd) const;

        void setTopic(const std::string &topic);
        void setPassword(const std::string &password);
        void setInviteOnly(bool inviteOnly);
		void setOperator(int);
        void setModerated(bool moderated);
        void setUserLimit(size_t limit);
        void removePassword();
        void toggleTopic();

        const std::string &getName() const;
        const std::string &getTopic() const;
        const std::vector<int> &getMembers() const;
        const std::string &getPassword() const ;
        bool getInviteOnly() const ;
        bool getModerated() const ;
		unsigned long getLimit() const ;
        size_t getUserLimit() const ;
        bool getTopicRestricted() const ;
        bool getProtected() const ;
        const std::vector<int> &getOperators() const ;
		void removeOperator(int clientFd);
    private:
        std::string _name;
        std::string _topic;
        std::string _password;
        std::vector<int> _operators; // Store client file descriptors of channel operators
        std::vector<int> _members; // Store client file descriptors of members
        std::vector<int> _invitedMembers; // Store client file descriptors of invited members

        bool _inviteOnly;
        bool _moderated;
        bool _topicRestricted;
        bool _protected;

        size_t _userLimit;


};
