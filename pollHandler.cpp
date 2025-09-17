#include "pollHandler.hpp"
#include "runCGI.hpp"

#include <cstdlib>
#include <ctime>

bool is_cgi_request(const Client & /*client*/) {
	//return std::rand() % 2 == 0;
	 return false;
} //TODO keep until request is parsed, thenthis will be moved to Request

void handle_new_connection(Server &server, std::vector<struct pollfd> &pfds, std::map<int, Client> &clients) {
	int new_client_fd= accept(server.get_fd(), NULL, NULL);
	if (new_client_fd < 0)
		perror("accept failed\n"); //OJO perror not allowed after reading or write
	//printf("File descriptor %d is ready to read\n", new_client_fd);
	clients.insert(std::make_pair(new_client_fd, Client(new_client_fd))); /// Not sure about this
	pfds.push_back(Server::create_pollfd(new_client_fd, POLLIN, 0));
}

void debug_request(Client& client) {
	if (!client.parse_request()) {
		std::cout << "Parse error: " << client.get_parse_error().code << std::endl;
		std::cout << "Parse error: " << client.get_parse_error().msg << std::endl;
    }    
    client.print_raw_request();

	std::cout << "DEBUG Host: " << client.get_header("Hot") << std::endl;
	std::cout << "DEBUG Error: " << client.get_parse_error().code ;
	std::cout << " " << client.get_parse_error().msg << std::endl;
	std::cout << "DEBUG Path: " << client.get_path() << std::endl;

    client.print_request_struct();
}

bool handle_client_read(int fd, std::vector<struct pollfd> &pfds, std::map<int, Client> &clients, size_t &index) {
	Client &client = clients[fd];
	char buffer[256];

	int bytes_read = read(fd, buffer, sizeof(buffer));
	if (bytes_read <= 0) {
		printf("Client %d disconnected\n", fd);
		if (!client.get_raw_request().empty()) {
            std::cout << "Attempting to parse incomplete request..." << std::endl;
            if (!client.parse_request()) {
                std::cout << "Parse error on disconnect: " << client.get_parse_error().code 
                          << " - " << client.get_parse_error().msg
						  << "Parse error passed to client: " << client.get_error_code() << std::endl;
                // Could send error response here if connection still writable
            }
        }
		return false;
	}
	client.add_to_request(buffer, bytes_read);
	if (client.is_read_complete()) {
		if (!client.parse_request()) { // Calling parser, will also set_error
			std::cout << "Parse error: " << client.get_parse_error().code << std::endl;
			std::cout << "Parse error: " << client.get_parse_error().msg << std::endl;
			std::cout << "Parse error passed to client: " << client.get_error_code() << std::endl;
			return false;
		}
		debug_request(client);
		pfds[index].events = POLLOUT;
	}
	return true;
}

// void handle_client_write(int fd) {
// 	const char *response = 	/// For now we return this string as response
// 		"HTTP/1.1 200 OK\r\n"
// 		"Content-Length: 12\r\n"
// 		"Content-Type: text/plain\r\n"
// 		"\r\n"
// 		"Hello world!";

// 	printf("=== Sending response to fd %d\n\n", fd);
// 	write(fd, response, strlen(response)); //TODO we need to write in chunks later
// }

void run_server(Server server) {
	std::vector<struct pollfd> pfds;
	std::map<int, Client> clients;

	pfds.push_back(Server::create_pollfd(server.get_fd(), POLLIN, 0));
	while (1)
	{
		int poll_count = poll(pfds.data(), pfds.size(), -1);
		if (poll_count == -1){
			perror("poll failed or no request\n");
			break;
		}
		for (size_t i = 0; i < pfds.size(); ++i) { //OJO something is wrong here. If we do not close after writing the number of fds will keep increasing infinitely
			
			Client &client = clients[pfds[i].fd];
			
			if (pfds[i].fd == server.get_fd() && pfds[i].revents & POLLIN)
			handle_new_connection(server, pfds, clients);
			if (pfds[i].fd != server.get_fd() && pfds[i].revents & POLLIN) {
				if (!handle_client_read(pfds[i].fd, pfds, clients, i)) { //OJO parser will be called here
					close(pfds[i].fd );
					clients.erase(pfds[i].fd);
					pfds.erase(pfds.begin() + i);
					--i;
					continue;
				}
			}
			else if (pfds[i].revents & POLLOUT && client.is_read_complete()) {
				if (is_cgi_request(client)) {
                    run_cgi("./www/cgi-bin/test.py", pfds[i].fd);
                }
				else // If not CGI, we've already set POLLOUT in handle_client_read
					handle_client_write(client, server.get_config());
	
				// This should only happen after client closes connection or timeout
				//The following will allow us to reuse the fds 
				// close(pfds[i].fd); //OJO we do not want to close the connection if still writting or cgi is running
				// clients.erase(pfds[i].fd); //wrong
				// pfds.erase(pfds.begin() + i);
				// --i;
			}
		}
	}
	close(server.get_fd());
}
