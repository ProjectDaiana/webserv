#include "server.hpp"

Server::Server(std::string ip_address, int port)
    : _port(port), _ip_address(ip_address)
{
    _sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock_fd < 0)
        throw std::runtime_error("Socket creation failed");

    memset(&_sock_addr, 0, sizeof(_sock_addr)); //TODO can we use memset/libft?
    _sock_addr.sin_family = AF_INET;
    _sock_addr.sin_port = htons(_port);
    // _sock_addr.sin_addr.s_addr = inet_addr(_ip_address.c_str());
	_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); //again, convert to network byte order but this time for a long instead of short. INADDR_ANY is 0.0.0.0. so any network interfaces are listened to
	
	int reuseadr = 1;
	signal(SIGPIPE, SIG_IGN);
	setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuseadr, sizeof(reuseadr));	//this is for setting the socket option to reuse the socket adress in memory. first the variable, then that its a _SO_cket _L_evel option, then that u should SOcket REUSEADRESS and finally a pointer to the int where the boolean about this is store and its size so that the ft knows how many bytes to read
	//using reuse adress option, makes it so that tcp doesnt wait for 1-4 minutes after closing the connection until we can use that adress again
	//binding the socket to an address and port, first init the struct member
	if (bind(_sock_fd, (struct sockaddr*)&_sock_addr, sizeof(_sock_addr)) < 0) {
		perror("bind fail\n");
		close(_sock_fd);
		throw std::runtime_error("Bind failed");
	}
	if (listen(_sock_fd, SOMAXCONN) < 0) { //start queueing incoming connections, SOMAXXCONN tells it to accept the maximum safe amount of client_fds
		perror("listen failed\n");
		close(_sock_fd);
		throw std::runtime_error("Listen failed");
	}

}

Server::~Server() {
	closeServer();
};

int Server::startServer(){
	_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock_fd < 0) {
		throw std::runtime_error("_sock_fd failed to init"); // check for the proper error
		return 1;
	}
	return 0;
};

int Server::closeServer(){
	if (_sock_fd >= 0) {
		close(_sock_fd);
		exit(0);
	}
	return 0;
}
