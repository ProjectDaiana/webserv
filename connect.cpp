/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connect.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 18:49:11 by ltreser           #+#    #+#             */
/*   Updated: 2025/08/21 20:51:55 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <csignal>

/*this is what the sockaddr_in struct looks like
struct sockaddr_in {
    short          sin_family;   // address family (AF_INET)
    unsigned short sin_port;     // port number (network byte order)
    struct in_addr sin_addr;     // IP address
    char           sin_zero[8];  // padding, ignore
};

in_addr struct:
struct in_addr {
    uint32_t s_addr;  // 32-bit IPv4 address in network byte order
};
*/


int	main(void)
{
	const char *response = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 12\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Hello world!";
	int sock; 
	int reuseadr;
	int	client;
	sockaddr_in addr;

	reuseadr = 1;
	addr = {};
	signal(SIGPIPE, SIG_IGN);	//if theres SIGPIPE signal (sent when trying 2 write 2 a closed pipe/socket) -> SIG_IGN aka signal ignore
	sock = socket(AF_INET, SOCK_STREAM, 0);	//system call to create a new socket, returns a fd integer handle, works then like a pipe to IP/TCP and u can read/write/bind/listen/accept. AF_INET is used for IPv4, internet protocol version 4 (32-bit adresses, eg. 127.0.0.1). AF stands for adress family. SOCK_STREAM -> stream is the type of socket, tcp is a stream socket.
	if (sock < 0)
		return (perror("socket failed to init\n"), 1); 
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseadr, sizeof(reuseadr));	//this is for setting the socket option to reuse the socket adress in memory. first the variable, then that its a _SO_cket _L_evel option, then that u should SOcket REUSEADRESS and finally a pointer to the int where the boolean about this is store and its size so that the ft knows how many bytes to read
	//using reuse adress option, makes it so that tcp doesnt wait for 1-4 minutes after closing the connection until we can use that adress again
	//binding the socket to an address and port, first init the struct member
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080); //on some hosts, the bytes for 8080 need to be swapped, this ft does that (convert to network byte order)
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //again, convert to network byte order but this time for a long instead of short. INADDR_ANY is 0.0.0.0. so any network interfaces are listened to
	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		return (perror("bind fail\n"), close(sock), 1);
	if (listen(sock, SOMAXCONN) < 0) //start queueing incoming connections, SOMAXXCONN tells it to accept the maximum safe amount of clients
		return (perror("listen failed\n"), close(sock), 1);
	while (1)
	{
		client = accept(sock, nullptr, nullptr);
		if (client < 0)
			return (perror("accept failed\n"), 1);
		write(client, response, strlen(response));
		close(client);
	}
	close(sock);
	return (0);
}	
		
