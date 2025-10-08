#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP
#include <map>
#include <vector>
#include "Server.hpp"
#include "Client.hpp"
#include "Request.hpp"

void	run_server(Server **servers, int server_count);
void	handle_new_connection(Server &server, std::vector<struct pollfd> &poll_fds, std::map<int, Client> &clients);
bool	handle_client_read(int fd, std::vector<struct pollfd> &poll_fds, std::map<int, Client> &clients, size_t &index);
void	handle_client_write(int fd);

#endif
