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
	private:
		int _fd;
		// sockaddr_in _address;
		
	public:
		Server(t_server *config);
		~Server();
		
		int closeServer();
		
		int get_fd();
		sockaddr_in get_sockaddr();
		
		//TODO move to private, have getters and setters
		static struct pollfd create_pollfd(int fd, short events, short revents);
};

#endif
