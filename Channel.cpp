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