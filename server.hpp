#ifndef SERVER_HPP
#define SERVER_HPP

#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <csignal>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class Server {

	public:
	Server(std::string ip_address, int port);
	~Server();
	
	int startServer();
	int closeServer();
	
	int _sock_fd;
	int _port;
	std::string _ip_address;
	struct sockaddr_in _sock_addr;

};

#endif