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
		t_server _config;
	public:
		Server() {};
		Server(t_server *config);
		~Server();
		// handle_finish_and_exit to avoid closing an fd before finishing writting
		int closeServer();
		
		int get_fd();
		sockaddr_in get_sockaddr();
		
		//TODO see if this function goes here or outside of the class
		static struct pollfd create_pollfd(int fd, short events, short revents);
		const t_server& get_config() const {return _config;}
};

#endif
