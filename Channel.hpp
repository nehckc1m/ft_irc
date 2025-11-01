#pragma once


#include <string>
#include <vector>
#include <map>


class Channel {
    public:
        Channel(const std::string &name);
        ~Channel();

        void addMember(int clientFd);
        void removeMember(int clientFd);
        bool isMember(int clientFd) const;

        void setTopic(const std::string &topic);
        void setPassword(const std::string &password);
        void setInviteOnly(bool inviteOnly);
        void setModerated(bool moderated);
        void setUserLimit(size_t limit);

        const std::string &getName() const;
        const std::string &getTopic() const;
        const std::vector<int> &getMembers() const;
    private:
        std::string _name;
        std::string _topic;
        std::string _password;

        std::vector<int> _operators; // Store client file descriptors of channel operators
        std::vector<int> _members; // Store client file descriptors of members
        bool _inviteOnly;
        bool _moderated;
        size_t _userLimit;


};
