#include "pollHandler.hpp"
#include "runCGI.hpp"
#include <cstdlib>
#include <ctime>

void handle_server_fd(pollfd &pfd, Server &server,
                      std::vector<pollfd> &pfds,
                      std::map<int, Client> &clients)
{
    if (pfd.revents & POLLIN)
    {
        int new_client_fd = accept(server.get_fd(), NULL, NULL);
        if (new_client_fd < 0)
        {
            perror("accept failed");
            return;
        }
        Client new_client(new_client_fd);
        new_client.update_activity();
        clients.insert(std::make_pair(new_client_fd, new_client));
        pfds.push_back(Server::create_pollfd(new_client_fd, POLLIN, 0));
    }
}

void	debug_request(Client &client)
{
	if (!client.parse_request())
	{
		std::cout << "Parse error: " << client.get_parse_error().code << std::endl;
		std::cout << "Parse error: " << client.get_parse_error().msg << std::endl;
	}
	client.print_raw_request();
	std::cout << "DEBUG Host: " << client.get_header("Host") << std::endl;
	std::cout << "DEBUG Error: " << client.get_parse_error().code;
	std::cout << " " << client.get_parse_error().msg << std::endl;
	std::cout << "DEBUG Path: " << client.get_path() << std::endl;
	client.print_request_struct();
}

bool	handle_client_read(int fd, pollfd &pfd, Client &client)
{
	char	buffer[256];
	int		bytes_read;

	client.update_activity();
	bytes_read = read(fd, buffer, sizeof(buffer));
	if (bytes_read <= 0)
	{
		printf("Client %d disconnected\n", fd);
		if (!client.get_raw_request().empty())
		{
			std::cout << "Attempting to parse incomplete request..." << std::endl;
			if (!client.parse_request())
			{
				std::cout << "Parse error on disconnect: " << client.get_parse_error().code << "- " << client.get_parse_error().msg << "Parse error passed to client: " << client.get_error_code() << std::endl;
				// Could send error response here if connection still writable
			}
		}
		return (false);
	}
	client.add_to_request(buffer, bytes_read);
	if (client.is_read_complete())
	{
		if (!client.parse_request())
		{ // Calling parser, will also set_error
			std::cout << "Parse error: " << client.get_parse_error().code << std::endl;
			std::cout << "Parse error: " << client.get_parse_error().msg << std::endl;
			std::cout << "Parse error passed to client: " << client.get_error_code() << std::endl;
			return (false);
		}
		debug_request(client);
		pfd.events = POLLOUT;
	}
	return (true);
}


void	add_server_sockets(Server **servers, int server_count, std::vector<struct pollfd> &pfds)
{
	int i;

	i = 0;
	while (i < server_count)
	{
		pfds.push_back(Server::create_pollfd((*servers)[i].get_fd(), POLLIN, 0));
		i++;
	}
}

int ft_poll(std::vector<struct pollfd>& pfds, int timeout_ms)
{
    int ret = poll(pfds.data(), pfds.size(), timeout_ms);
    if (ret == -1)
        perror("poll failed");
    return ret;
}

Server* find_server(int fd, Server** servers, int server_count)
{
    int i = 0;
    while (i < server_count)
    {
        if (fd == (*servers)[i].get_fd())
            return &(*servers)[i]; // return pointer to the server
        i++;
    }
    return NULL; // not found
}

void close_servers(Server **servers, int server_count)
{
    int i = 0;
    while (i < server_count)
    {
        close((*servers)[i].get_fd());
        i++;
    }
}

void cleanup_client(int fd, std::vector<pollfd> &pfds, std::map<int, Client> &clients)
{
    close(fd);
    clients.erase(fd);

    size_t i = 0;
    while (i < pfds.size())
    {
        if (pfds[i].fd == fd)
        {
            pfds.erase(pfds.begin() + i);
            break;
        }
        i++;
    }
}


t_server* find_server_for_client(int client_fd, const std::map<int, Client> &clients,
                                 Server **servers, int server_count)
{
    std::map<int, Client>::const_iterator it = clients.find(client_fd); //find client
    if (it == clients.end())
        return NULL;
    const Client &client = it->second;
    int i = 0;
    while (i < server_count) 
	{
        if (client.get_listen_fd() == servers[i]->get_fd()) //check if client was accepted from this fd
		//TODO store listen fd in client, getter & setter
            return &(servers[i]->get_config());
        i++;
    }
    return NULL;
}

void handle_client_fd(pollfd &pfd, std::vector<pollfd> &pfds, std::map<int, Client> &clients, const t_server &server_config)
{
    Client &client = clients[pfd.fd];
    time_t now = std::time(NULL);

    // TIMEOUT CHECK
    const int TIMEOUT = 120; // seconds
    if (now - client.get_last_activity() > TIMEOUT)
    {
        printf("Client %d timed out\n", pfd.fd);
        cleanup_client(pfd.fd, pfds, clients);
        return;
    }

    // READ
    if (pfd.revents & POLLIN)
    {
        if (!handle_client_read(pfd.fd, pfd, client))
        {
            cleanup_client(pfd.fd, pfds, clients);
            return;
        }
    }
    // WRITE
    else if (pfd.revents & POLLOUT && client.is_read_complete())
    {
        if (client.is_cgi())
            run_cgi("./www/cgi-bin/test.py", pfd.fd);
        else
            handle_client_write(client, server_config);

        if (client.is_write_complete())
        {
            client.reset();
            pfd.events = POLLIN; // go back to reading
        }
    }
}


void    run_server(Server** servers, int server_count)
{
    std::vector<struct pollfd> pfds;
    std::map<int, Client> clients;
   // const int TIMEOUT = 120; // seconds
	//int		poll_count;
	//time_t	now;
	size_t i = 0;

	add_server_sockets(servers, server_count, pfds);
    while (1)
    {
        if (ft_poll(pfds, 1000) == -1)
			break;
		i = 0;
		
        while (i < pfds.size())
        {
			Server *server = find_server(pfds[i].fd, servers, server_count);
			if (server)
				handle_server_fd(pfds[i], *server, pfds, clients);
			else
				handle_client_fd(pfds[i], pfds, clients, *find_server_for_client(pfds[i].fd, clients, server, server_count));
		i++;
		}
	}
	close_servers(servers, server_count);
}



















         /*   if (pfds[i].fd != (*servers)[0].get_fd())
            {
                Client &client = clients[pfds[i].fd];
                //now = std::time(NULL);
                // Timeout check
                // TODO also close if client closed connection
                if ((now - client.get_last_activity() > TIMEOUT))
                {
                    std::time_t result = client.get_last_activity();
                    printf("XXXXXX Client %d timed out, closing connection XXXXXX\n", pfds[i].fd);
					std::cout << "XXXXXX Now: " << std::asctime(std::localtime(&now));
					std::cout << "XXXXXX Last activity client " << client.get_fd() << ": " << std::asctime(std::localtime(&result));
                    std::cout << "XXXXXX Connection: " << client.get_header("Connection") << std::endl;
                    // TODO move to a cleanup function
                    close(pfds[i].fd);
                    clients.erase(pfds[i].fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue ;
                }
                // Handle client read
                if (pfds[i].revents & POLLIN)
                {
                    if (!handle_client_read(pfds[i].fd, pfds, clients, i))
                    { // OJO parser will be called here
                        // TODO move to a cleanup function
                        close(pfds[i].fd);
                        clients.erase(pfds[i].fd);
                        pfds.erase(pfds.begin() + i);
                        --i;
                        continue ;
                    }
                }
                // Handle client write
                else if (pfds[i].revents & POLLOUT && client.is_read_complete())
                {
                    if (client.is_cgi())
                        run_cgi("./www/cgi-bin/test.py", pfds[i].fd);
                    else
                        handle_client_write(client, (*servers)[0].get_config());
                    if (client.is_write_complete())
                    {
                        client.reset();
                        pfds[i].events = POLLIN;
                    }
                }
            }
            i++;
        }
    }
    close((*servers)[0].get_fd());
}*/


/*void	run_server(Server server)
{
	size_t	i;
	int		poll_count;
	time_t	now;

	std::vector<struct pollfd> pfds;
	std::map<int, Client> clients;
	const int TIMEOUT = 120; // seconds
	i = 0;
	pfds.push_back(Server::create_pollfd(server.get_fd(), POLLIN, 0));
	while (1)
	{
		poll_count = poll(pfds.data(), pfds.size(), 1000);
		if (poll_count == -1)
		{
			perror("poll failed or no request\n");
				// TODO replace for proper error
			break ;
		}
		while (i < pfds.size())
		{
			if (pfds[i].fd == server.get_fd() && pfds[i].revents & POLLIN)
				handle_new_connection(server, pfds, clients);
			if (pfds[i].fd != server.get_fd())
			{
				Client &client = clients[pfds[i].fd];
				now = std::time(NULL);
				// Timeout check
				// TODO also close if client closed connection
				if ((now - client.get_last_activity() > TIMEOUT))
				{
					std::time_t result = client.get_last_activity();
					printf("XXXXXX Client %d timed out,
						closing connection XXXXXX\n", pfds[i].fd);
					std::cout << "XXXXXX Now: " << std::asctime(std::localtime(&now));
					std::cout << "XXXXXX Last activity client " << client.get_fd() << ": " << std::asctime(std::localtime(&result));
					std::cout << "XXXXXX Connection: " << client.get_header("Connection") << std::endl;
					// TODO move to a cleanup function
					close(pfds[i].fd);
					clients.erase(pfds[i].fd);
					pfds.erase(pfds.begin() + i);
					--i;
					continue ;
				}
				// Handle client read
				if (pfds[i].revents & POLLIN)
				{
					if (!handle_client_read(pfds[i].fd, pfds, clients, i))
					{ // OJO parser will be called here
						// TODO move to a cleanup function
						close(pfds[i].fd);
						clients.erase(pfds[i].fd);
						pfds.erase(pfds.begin() + i);
						--i;
						continue ;
					}
				}
				// Handle client write
				else if (pfds[i].revents & POLLOUT && client.is_read_complete())
				{
					if (client.is_cgi())
						run_cgi("./www/cgi-bin/test.py", pfds[i].fd);
					else
						handle_client_write(client, server.get_config());
					if (client.is_write_complete())
					{
						client.reset();
						pfds[i].events = POLLIN;
					}
				}
			}
			i++;
		}
	}
	close(server.get_fd());
}*/
