#include "Server.hpp"

Server::Server(int port, const std::string &password)
    : _port(port), _password(password) {
}

Server::~Server() {
    // Close server socket and clean up resources if needed
    
}

void Server::createSocket() {
       _servSocket = initSocket();
       bindAndListen(_servSocket);
       addSocketToPoll(_servSocket);
       std::cout << "Server socket created on port " << this->getPort() << std::endl;
}

int Server::initSocket() {
       int sockFd = socket(AF_INET, SOCK_STREAM, 0);
       if (sockFd == -1) {
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
              throw std::runtime_error("Failed to bind socket: " + std::string(std::strerror(errno)));
       }
       if (listen(socket, SOMAXCONN) == -1) {
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

void Server::acceptNewConnection() {
       struct sockaddr_in client_addr;
       socklen_t client_len = sizeof(client_addr);
       int client_socket = accept(_servSocket, (struct sockaddr *)&client_addr, &client_len);
       if (client_socket == -1) {
              std::cerr << "Failed to accept new connection" << std::endl;
              return;
       }
       // Set client socket to non-blocking mode
       if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1) {
              std::cerr << "Failed to set client socket to non-blocking" << std::endl;
              close(client_socket);
              return;
       }
       Client newClient(client_socket);
       clients.push_back(newClient);
       addSocketToPoll(client_socket);
       std::cout << "New client connected of fd: " << client_socket << std::endl;
       sendMessage(client_socket, "STOP RIGHT THERE THIS IS GOTTA GO FAST IRC SERVER!\r\nPlease authenticate using PASS <password>\r\n");
}

void Server::run(){
       std::cout << "Server is running. Waiting for events..." << std::endl;

       while (!_signal) {
              int ret = poll(poll_fds.data(), poll_fds.size(), -1);

              if (ret == -1 && !_signal) {
                     std::cerr << "poll() error" << std::endl;
                     break;
              }
              for (size_t i = 0; i < poll_fds.size(); ++i) {
                     if (poll_fds[i].revents & POLLIN) {
                            if (poll_fds[i].fd == _servSocket) {
                                   acceptNewConnection();
                            } else {
                                   char buffer[1024];
                                   int bytes = recv(poll_fds[i].fd, buffer, sizeof(buffer), 0);
                                   if (bytes >= 1)
                                          processBuffer(poll_fds[i].fd, buffer, bytes);
                                   if (bytes <= 0) {
                                          removeClient(poll_fds[i].fd);
                                          --i;
                                   }
                            }
                     }
                     if (poll_fds[i].revents & POLLOUT) {
                            sendMessage(poll_fds[i].fd, "");
                     }
              }
       }
}

void Server::removeClient(int clientFd)
{
       clientBuffers.erase(clientFd);
       for (size_t i = 0; i < clients.size(); ++i)
       {
              if (clients[i].getFd() == clientFd)
              {
                     std::cout << "Removing client with fd: " << clientFd << std::endl;
                     close(clientFd);
                     clients.erase(clients.begin() + i);
                     break;
              }
       }
       for (size_t i = 0; i < poll_fds.size(); ++i)
       {
              if (poll_fds[i].fd == clientFd)
              {
                     poll_fds.erase(poll_fds.begin() + i);
                     break;
              }
       }

}

int Server::getClientFdByNickname(const std::string &nickname) const {
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i].getNickname() == nickname)
			return clients[i].getFd();
	}
	return -1; // not found
}