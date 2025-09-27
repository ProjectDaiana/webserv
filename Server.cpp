#include "Server.hpp"
#include "webserv.hpp"


//NOTE there can be several servers, they only have one port & one ip each
//TODO replace w return value management later, so that other clean up can happen
Server::Server(t_server *config)
{
	sockaddr_in address = {};
	int reuseadr = 1;

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0 && (perror("Socket creation failed\n"), 1)) //TODO make nicer, helper
		exit(0);	
	address.sin_family = AF_INET;
	address.sin_port = htons(config->lb->port);
	address.sin_addr.s_addr = iptoi(config->lb->host);
	signal(SIGPIPE, SIG_IGN);
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &reuseadr, sizeof(reuseadr));
	if (bind(_fd, (struct sockaddr*)&address, sizeof(address)) < 0 && (perror ("Bind failed\n"), 1)) 
		exit(0);
	if (listen(_fd, SOMAXCONN) < 0 && (perror("Listen failed\n"), 1))
		exit(0);
	_config = *config;
}

Server::~Server() {
	closeServer();
};


int Server::closeServer(){
	if (_fd >= 0) {
		close(_fd);
		exit(0);
	}
	return 0;
}

struct pollfd Server::create_pollfd(int fd, short events, short revents) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = revents;
    return pfd;
}

int Server::get_fd() const {
	return _fd;
}

// sockaddr_in Server::get_sockaddr() {
// 	return _address;
// }
