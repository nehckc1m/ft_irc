#include "Channel.hpp"

Channel::Channel(const std::string &name) : _name(name), _topic (""), _password("") {
    std::cout << "Channel " << name << " created." << std::endl;
}

Channel::~Channel() {
    std::cout << "Channel " << _name << " destroyed." << std::endl;
}

void Channel::addMember(int clientFd) {
    if (!isMember(clientFd)) {
        _members.push_back(clientFd);
    }
}

void Channel::removeMember(int clientFd) {
    std::vector<int>::iterator it = std::find(_members.begin(), _members.end(), clientFd);
    if (it != _members.end()) {
        _members.erase(it);
    }
}


bool Channel::isMember(int clientFd) const {
    return std::find(_members.begin(), _members.end(), clientFd) != _members.end();
}
const std::vector<int> &Channel::getMembers() const {
    return _members;
}
const std::string &Channel::getName() const {
    return _name;
}

void Channel::removePassword() {
    _password.clear();
}

void Channel::setInviteOnly(bool inviteOnly) {
    _inviteOnly = inviteOnly;
}

void Channel::setModerated(bool moderated) {
    _moderated = moderated;
}

void Channel::setUserLimit(size_t limit) {
    _userLimit = limit;
}

void Channel::setPassword(const std::string &password) {
    _password = password;
}

void Channel::setTopic(const std::string &topic) {
    _topic = topic;
}