#include "server.hpp"


//TODO rewrite server constructor, storing only relevant runtime data
//NOTE there can be several servers, they only have one port & one ip each
//TODO replace w return value management later, so that other clean up can happen
Server::Server(t_server *config)
{
	sockaddr_in address = {};
	int reuseadr = 1;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0 && perror("Socket creation failed\n"))
		exit(0);	
	address.sin_family = AF_INET;
	address.sin_port = htons(config->lb->port);
	//TODO implement helper fts: import atoi helper, create a ip_to_uint32 helper ft for this case to convert the char * str to network order 
	//address.sin_addr.s_addr = htonl(config->lb->host);
	signal(SIGPIPE, SIG_IGN);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseadr, sizeof(reuseadr));
	if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0 && perror ("Bind failed\n")) 
		exit(0);
	if (listen(_sock_fd, SOMAXCONN) < 0 && perror("Listen failed\n"))
		exit(0);
}

Server::~Server() {
	closeServer();
};


int Server::closeServer(){
	if (_sock_fd >= 0) {
		close(_sock_fd);
		exit(0);
	}
	return 0;
}
