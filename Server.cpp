#include "Server.hpp"


void Server::createSocket() {
       _servSocket = initSocket();
       bindAndListen(_servSocket);
       addSocketToPoll(_servSocket);
       std::cout << "Server socket created on port " << this->getPort() << std::endl;
}


int Server::initSocket() {
       int sockFd = socket(AF_INET, SOCK_STREAM, 0);
       if (sockFd == -1) {
              close(sockFd);
              throw std::runtime_error("Failed to create socket");
       }
       // Set socket to non-blocking mode
       if (fcntl(sockFd, F_SETFL, O_NONBLOCK) == -1)
              throw std::runtime_error("Failed to set socket to non-blocking");
       int opt = 1;
       if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
       {
              close(sockFd);
              throw std::runtime_error("Failed to set socket options");
       }
       return sockFd;
}

void Server::bindAndListen(int socket) {
       struct sockaddr_in serv_adress;
       serv_adress.sin_family = AF_INET;
       serv_adress.sin_addr.s_addr = INADDR_ANY;
       serv_adress.sin_port = htons(this->getPort());

       if (bind(socket, (struct sockaddr *)&serv_adress, sizeof(serv_adress)) == -1) {
              close(socket);
              throw std::runtime_error("Failed to bind socket");
       }
       if (listen(socket, 10) == -1) {
              close(socket);
              throw std::runtime_error("Failed to listen on socket");
       }
}

void Server::addSocketToPoll(int socket) {
       struct pollfd pFd;
       pFd.fd = socket;
       pFd.events = POLLIN;
       pFd.revents = 0;
       poll_fds.push_back(pFd);
}