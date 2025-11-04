#include "Server.hpp"
#include "webserv.hpp"


Server::Server(t_data *d, t_server *config)
{
	sockaddr_in address = {};
	int reuseadr = 1;

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		ft_error(d->perm_memory, SOCKET_CREATION_FAILED, 1);
	address.sin_family = AF_INET;
	address.sin_port = htons(config->lb->port);
	address.sin_addr.s_addr = iptoi(config->lb->host);
	signal(SIGPIPE, SIG_IGN);
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &reuseadr, sizeof(reuseadr));
	if (bind(_fd, (struct sockaddr*)&address, sizeof(address)) < 0) 
		ft_error (d->perm_memory, BIND_FAILED, 1);
	if (listen(_fd, SOMAXCONN) < 0)
		ft_error(d->perm_memory, LISTEN_FAILED, 1);
	_config = *config;
}

Server::~Server() {
	closeServer();
};


int Server::closeServer(){
	if (_fd >= 0)
	{ 
		close(_fd);
		//exit(0); DEL
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
