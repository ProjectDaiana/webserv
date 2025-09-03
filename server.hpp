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
#include "webserv.hpp"

//NOTE this server only contains runtime data, any static data is stored by the config data

class Server {

	public:
	Server(t_server *config);
	~Server();
	
	int closeServer();
//TODO move to private, have getters and setters
	int fd;
	sockaddr_in address;
	static struct pollfd create_pollfd(int fd, short events, short revents);
};

#endif
