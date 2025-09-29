#include "pollHandler.hpp"
#include "runCGI.hpp"
#include <cstdlib>
#include <ctime>

bool is_cgi_request(const Client & /*client*/) {
	//return std::rand() % 2 == 0;
	 return false;
} //TODO keep until request is parsed, thenthis will be moved to Request

// Client* find_cgi_client(int pipe_fd, std::map<int, Client> &clients) {
//     for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
//         if (it->second.get_cgi_pipe() == pipe_fd) {
//             return &it->second;
//         }
//     }
//     return NULL;
// }

void handle_new_connection(Server &server, std::vector<struct pollfd> &pfds, std::map<int, Client> &clients) {
	int new_client_fd= accept(server.get_fd(), NULL, NULL);
	if (new_client_fd < 0){
		perror("accept failed\n"); //OJO perror not allowed after reading or write
		return;
	}
	//printf("File descriptor %d is ready to read\n", new_client_fd);
	Client new_client(new_client_fd);
	std::cout << "New Connection, client fd: " << new_client_fd 
              << " stored fd: " << new_client.get_fd() 
              << " activity: " << new_client.get_last_activity() << std::endl;
	new_client.update_activity();
	clients.insert(std::make_pair(new_client_fd, new_client));
	pfds.push_back(Server::create_pollfd(new_client_fd, POLLIN, 0));
}

void debug_request(Client& client) {
	if (!client.parse_request()) {
		std::cout << "Parse error: " << client.get_parse_error().code << std::endl;
		std::cout << "Parse error: " << client.get_parse_error().msg << std::endl;
    }    
    //client.print_raw_request();

	std::cout << "DEBUG Host: " << client.get_header("Host") << std::endl;
	std::cout << "DEBUG Error: " << client.get_parse_error().code ;
	std::cout << " " << client.get_parse_error().msg << std::endl;
	std::cout << "DEBUG Path: " << client.get_path() << std::endl;

//    client.print_request_struct();
}

bool handle_client_read(int fd, std::vector<struct pollfd> &pfds, std::map<int, Client> &clients, size_t &i, std::map<int, Client*> &cgi_pipes) {
	Client &client = clients[fd];
	client.update_activity();
	char buffer[256];

	int bytes_read = read(fd, buffer, sizeof(buffer));
	if (bytes_read <= 0) {
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
		if (client.is_cgi()) {
		    // Initiate the CGI process and disable polling on the client's socket for now.
		    run_cgi("./www/cgi-bin/test.py", client, pfds, cgi_pipes);
		    pfds[i].events = 0; //stop poollin pollout, im reading 
			client.set_cgi_running(1);
		} 
		else
		pfds[i].events = POLLOUT;
	}
	return true;
}

void run_server(Server server) {
	std::vector<struct pollfd> pfds;
	std::map<int, Client> clients;
	std::map<int, Client*> cgi_pipes;// chek if fds are correct
	const int TIMEOUT = 120; // seconds

	pfds.push_back(Server::create_pollfd(server.get_fd(), POLLIN, 0));
	while (1)
	{
		int poll_count = poll(pfds.data(), pfds.size(), 1000);	
		if (poll_count == -1){
			perror("poll failed or no request\n"); //TODO replace for proper error
			break;
		}
		for (size_t i = 0; i < pfds.size(); ++i) {

			if (pfds[i].fd == server.get_fd() && pfds[i].revents & POLLIN)
				handle_new_connection(server, pfds, clients);


			if (pfds[i].fd != server.get_fd()) {
				Client &client = clients[pfds[i].fd];
				time_t now = std::time(NULL);
				
				// Timeout check
				//TODO also close if client closed connection
				if ((now - client.get_last_activity() > TIMEOUT)) {
					std::time_t result =  client.get_last_activity() ;
					printf("XXXXXX Client %d timed out, closing connection XXXXXX\n", pfds[i].fd);
					std::cout << "XXXXXX Now: " <<  std::asctime(std::localtime(&now));
					std::cout << "XXXXXX Last activity client " << client.get_fd() << ": " << std::asctime(std::localtime(&result));
					std::cout << "XXXXXX Connection: " << client.get_header("Connection") << std::endl;

					//TODO move to a cleanup function
					close(pfds[i].fd);
					clients.erase(pfds[i].fd);
					pfds.erase(pfds.begin() + i);
					--i;
					continue;
				}
				// Handle client read
				if (pfds[i].revents & POLLIN) {
					if (!handle_client_read(pfds[i].fd, pfds, clients, i, cgi_pipes)) { //OJO parser will be called here
						//TODO move to a cleanup function
						close(pfds[i].fd );
						clients.erase(pfds[i].fd);
						pfds.erase(pfds.begin() + i);
						--i;

						continue;
					}
				}
							// cgi 
					if (client.is_cgi_running() && client.is_cgi()) {
						if (handle_cgi_write(client.get_cgi_pipe() , client)) {
							std::cout << "handle_write ok pid:"<<  client.get_cgi_pid() <<  std::endl;
							std::cout << "handle_write client fd:"<<  client.get_fd() <<  std::endl;		
							
							int client_fd = client.get_fd();
							// cgi_pipes.erase(pfds[i].fd);
							// // Remove CGI pipe fd from pfds
							// pfds.erase(pfds.begin() + i);
							// --i;
						
						// Re-enable the client socket for writing (search after we've modified pfds)
						for (size_t j = 0; j < pfds.size(); ++j) {
							if (pfds[j].fd == client_fd) {
								pfds[j].events = POLLOUT;
								break;
							}
					}
				}
				continue;
			}
				// Handle client write
				if (pfds[i].revents & POLLOUT && client.is_read_complete()) {
						handle_client_write(client, server.get_config());
					if (client.is_write_complete()) {
						client.reset();
						pfds[i].events = POLLIN;
					}
				}
			}
		}
	}
	close(server.get_fd());
}

