#include "Channel.hpp"

Channel::Channel(const std::string &name){
    _name = name;
    _topic = "";
    _password = "";
    _inviteOnly = false;
    _moderated = false;
    _topicRestricted = false;
    _protected = false;
    _userLimit = 0;
	_members = std::vector<int>();
	_invitedMembers = std::vector<int>();
	_operators = std::vector<int>();
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
	std::vector<int>::const_iterator it;
	for (it = _invitedMembers.begin(); it != _invitedMembers.end(); ++it) {
		if (*it == clientFd)
			return true;
	}
	return false;
}

void Channel::addMember(int clientFd) {
    if (!isMember(clientFd)) {
        _members.push_back(clientFd);
    }
}

void Channel::removeMember(int clientFd) {
    std::vector<int>::iterator it;
    for(it = _members.begin(); it != _members.end(); ++it) {
		if (*it == clientFd) {
			_members.erase(it);
			break;
		}
	}
}

void Channel::removeInvitedMember(int clientFd) {
	for (std::vector<int>::iterator it = _invitedMembers.begin(); it != _invitedMembers.end(); ++it) {
		if (*it == clientFd) {
			_invitedMembers.erase(it);
			break;
		}
	}
}

void Channel::addInvitedMember(int clientFd) {
	if (!isInvited(clientFd)) {
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
    std::vector<int>::const_iterator it;
	for (it = _members.begin(); it != _members.end(); ++it) {
		if (*it == clientFd)
			return true;
	}
	return false;
}
const std::vector<int> &Channel::getMembers() const {
    return _members;
}
const std::string &Channel::getName() const {
    return _name;
}

void Channel::removePassword() {
    _password.clear();
    _protected = false;
    //_password = "";
}

void Channel::setInviteOnly(bool inviteOnly) {
    _inviteOnly = inviteOnly;
}

void Channel::addOperator(int clientFd) {
    if (!isOperator(clientFd)) 
        _operators.push_back(clientFd);
}

bool Channel::isOperator(int clientFd) const {
    std::vector<int>::const_iterator it;
	for (it = _operators.begin(); it != _operators.end(); ++it) {
		if (*it == clientFd)
			return true;
	}
	return false;
}
const std::vector<int> &Channel::getOperators() const {
	return _operators;
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
	_protected = true;
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

unsigned long Channel::getLimit() const {
	return static_cast<unsigned long>(_userLimit);
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

