#include "Channel.hpp"

Channel::Channel(const std::string &name){
    _name = name;
    _topic = "";
    _password = "";
    _inviteOnly = true;
    _moderated = false;
    _topicRestricted = false;
    _protected = false;
    _userLimit = 0;
	_operators = std::vector<int>();
    std::cout << "Channel " << name << " created." << std::endl;
}

Channel& Channel::operator=(const Channel &other) {
    if (this != &other) {
        _name = other._name;
        _topic = other._topic;
        _password = other._password;
        _operators = other._operators;
        _members = other._members;
        _invitedMembers = other._invitedMembers;
        _inviteOnly = other._inviteOnly;
        _moderated = other._moderated;
        _topicRestricted = other._topicRestricted;
        _protected = other._protected;
        _userLimit = other._userLimit;
    }
    return *this;
}

Channel::~Channel() {

}

bool Channel::isInvited(int clientFd) const {
    return std::find(_invitedMembers.begin(), _invitedMembers.end(), clientFd) != _invitedMembers.end();
}

void Channel::addMember(int clientFd) {
    if (!isMember(clientFd)) {
        _members.push_back(clientFd);
    }
}

void Channel::addOperator(int clientFd) {
    if (!isOperator(clientFd)) 
        _operators.push_back(clientFd);
}

bool Channel::isOperator(int clientFd) const {
    return std::find(_operators.begin(), _operators.end(), clientFd) != _operators.end();
}

void Channel::removeMember(int clientFd) {
    std::vector<int>::iterator it = std::find(_members.begin(), _members.end(), clientFd);
    if (it != _members.end()) {
        _members.erase(it);
    }
}

void Channel::addInvitedMember(int clientFd) {
    if (!isMember(clientFd)) {
        _invitedMembers.push_back(clientFd);
    }
}

bool Channel::getProtected() const {
    return _protected;
}

void Channel::toggleTopic() {
    _topicRestricted = !_topicRestricted;
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

void Channel::setOperator(int clientFd) {
	if (clientFd == -1)
		return;
	if (!isOperator(clientFd))
		_operators.push_back(clientFd);

}

void Channel::removeOperator(int clientFd) {
	std::vector<int>::iterator it = std::find(_operators.begin(), _operators.end(), clientFd);
	if (it != _operators.end()) {
		_operators.erase(it);
	}
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

const std::string &Channel::getTopic() const {
    return _topic;
}

bool Channel::getInviteOnly() const {
    return _inviteOnly;
}
bool Channel::getModerated() const {
    return _moderated;
}

bool Channel::getTopicRestricted() const {
    return _topicRestricted;
}
size_t Channel::getUserLimit() const {
	return _userLimit;
}
const std::string &Channel::getPassword() const {
	return _password;
}
const std::vector<int> &Channel::getOperators() const {
	return _operators;
}
