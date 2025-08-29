#include "server.hpp"

// sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
// sendbytes = strlen(sendline);

int main (){
	Server server("0.0.0.0", 8080);
	std::vector<struct pollfd> poll_fds;
	int client_fd;
	const char *response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 12\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"Hello world!";

	poll_fds.push_back({server._sock_fd, POLLIN, 0}); 
	while (1)
	{
		// int poll(struct pollfd fds[], nfds_t nfds, int timeout);
		int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
		if (poll_count < 0){
			perror("poll failed or no request\n");
			return 1;
		}
		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			if (poll_fds[i].fd == server._sock_fd && poll_fds[i].revents & POLLIN)
			{
				printf("File descriptor %d is ready to read\n", poll_fds[i].fd);
				client_fd = accept(server._sock_fd, nullptr, nullptr);
				if (client_fd < 0)
					return (perror("accept failed\n"), 1);

				poll_fds.push_back({client_fd, POLLIN | POLLOUT, 0});
				//Now we need to read from poll_fds 
				// We need to route to static or cgi
				// when all read we set POLLOUT to the fd or initialice a process
			}
			else if (poll_fds[i].revents & POLLOUT)
			{
				printf("File descriptor %d is ready to write\n", poll_fds[i].fd);
				write(poll_fds[i].fd, response, strlen(response));
				//The following will allow us to reuse the fds
				close(poll_fds[i].fd); 
				poll_fds.erase(poll_fds.begin() + i);
				--i;
			}
		}
	}
	close(server._sock_fd);

	return 0;
}
