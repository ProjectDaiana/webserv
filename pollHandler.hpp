#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP
#include <map>
#include <vector>
#include "Server.hpp"
#include "Client.hpp"
#include "Request.hpp"

// test_idle_seconds: if >0, exit the server when the whole server has been idle
// for that many seconds (useful for leak testing). If -1 (default), server
// runs indefinitely. poll_timeout_ms controls the poll() timeout in ms and
// should be smaller than CGI timeout so CGI timeouts are enforced periodically.
void	run_server(Server **servers, int server_count, int idle_seconds = 30, int poll_timeout_ms = 1000);
void	handle_new_connection(Server &server, std::vector<struct pollfd> &poll_fds, std::map<int, Client> &clients);
bool	handle_client_read(int fd, std::vector<struct pollfd> &poll_fds, std::map<int, Client> &clients, size_t &index);
void	handle_client_write(int fd);

#endif
