#ifndef RUNCGI_HPP
#define RUNCGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <errno.h>
#include <signal.h>
#include "webserv.hpp"
#include "errors.hpp"

void remove_fd_from_poll(int fd, std::vector<struct pollfd>& pfds);
int run_cgi(Client& client, std::vector<struct pollfd>& pfds);
bool cgi_eof(int pipe_fd, Client &client, std::vector<struct pollfd>& pfds);
bool handle_cgi_read_from_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds);
bool check_cgi_timeout(Client& client, int timeout);
bool handle_cgi_timeout(Client& client);
bool handle_cgi_write_to_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds);

#endif
