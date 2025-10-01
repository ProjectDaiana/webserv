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
        Client new_client(new_client_fd, server);
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
		if (!client.is_cgi()) 
		//{
		  //  run_cgi("./www/cgi-bin/test.py", client, pfds, cgi_pipes);
		   // pfds[i].events = 0; //stop poollin pollout, im reading 
	//		client.set_cgi_running(1);
//		} 
//		else
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

void cleanup_client(int fd, std::vector<pollfd> &pfds, std::map<int, Client> &clients)
{
    printf("Client '%d' is being cleaned up\n", fd);
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

int ft_poll(std::vector<struct pollfd>& pfds, int timeout_ms, std::map<int, Client> clients)
{
	int ret = poll(pfds.data(), pfds.size(), timeout_ms);
	if (ret == -1)
        perror("poll failed");
	if (ret == 0)
	{
		time_t now = std::time(NULL);
		std::map<int, Client>::iterator i = clients.begin();
		while (i != clients.end())
		{
			if (now - i->second.get_last_activity() > 120)
			{
				int fd = i->first;
				i++;
				cleanup_client(fd, pfds, clients);
			}
			else
				i++;
		}
	}
	return ret;
}

Server* is_server(int fd, Server** servers, int server_count)
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


int find_client_for_cgi(int cgi_fd, const std::map<int, Client> &clients)
{
    std::map<int, Client>::const_iterator i = clients.begin();
    while (i != clients.end())
    {
        if (i->second.get_cgi_pipe() == cgi_fd)
            return i->first;
        i++;
    }
    return -1;
}

int is_cgi_fd(int fd, const std::map<int, Client> &clients)
{
    std::map<int, Client>::const_iterator it = clients.begin();
    while (it != clients.end())
    {
        if (it->first == fd)
            return 0; // fd is a normal client
        ++it;
    }
    return 1; // fd is cgi
}

Client& find_client(int fd, std::map<int, Client> &clients)
{
        if (is_cgi_fd(fd, clients))
        {
               int client_fd = find_client_for_cgi(fd, clients);
               return clients.at(client_fd);
	}
        else
                return clients.at(fd);
}

const t_server* find_server_for_client(int client_fd, const std::map<int, Client> &clients,
                                 Server **servers, int server_count)
{
	if (is_cgi_fd(client_fd, clients))
		client_fd = find_client_for_cgi(client_fd, clients);
	std::map<int, Client>::const_iterator it = clients.find(client_fd); //find client
	if (it == clients.end())
		return NULL;
	const Client &client = it->second;
    	int i = 0;
	while (i < server_count) 
	{
		if (client.get_server()->get_fd() == servers[i]->get_fd()) //check if client was accepted from this fd
			return &(servers[i]->get_config());
		i++;
	}
	return NULL;
}

int timeout_check(Client &client, int fd, std::vector<pollfd> &pfds, std::map<int, Client> &clients)
{
	time_t now = std::time(NULL);
	const int TIMEOUT = 120; // seconds
	if (now - client.get_last_activity() > TIMEOUT)
	{
		printf("Client %d timed out\n", fd);
		cleanup_client(fd, pfds, clients);
		return 0;
	}
	return 1;
}

/*
void handle_cgi(Client &client, const t_server &server_config, std::vector<pollfd> &pfds, pollfd &pfd)
{
	if (!client.is_cgi_running())
		run_cgi(client); //setup polls etc
	//init buffer for cgi reading
	read_cgi_output(client); //read
	handle_client_write(client, server_config); 
}*/

void set_client_pollout(std::vector<pollfd> &pfds, Client &client)
{
	 int client_fd = client.get_fd();
         for (size_t j = 0; j < pfds.size(); ++j)  //does iterator actually change?
         {
                if (pfds[j].fd == client_fd)
                {
                     pfds[j].events = POLLOUT;
                     break;
                 }
         }

}

void cleanup_cgi(std::vector<pollfd> &pfds, pollfd &pfd, Client &client)
{
	cgi_eof(pfd.fd, client);
        size_t i = 0;
        while (i < pfds.size())
        {
             if (pfds[i].fd == pfd.fd)
             {
                  pfds.erase(pfds.begin() + i);
                  break;
              }
              i++;
         }
	set_client_pollout(pfds, client);
}

int handle_client_fd(pollfd &pfd, std::vector<pollfd> &pfds, std::map<int, Client> &clients, const t_server &server_config)
{
	//write(1, "NO SGF YET\n", 11);
	printf("HANDLING PFD NR: '%d' now!\n", pfd.fd);
	if (pfd.revents & POLLIN)
		printf("POLLIN\n");
	if (pfd.revents & POLLOUT)
		printf("POLLOUT\n");
	printf("fd %d revents = 0x%x\n", pfd.fd, pfd.revents);
	Client &client = find_client(pfd.fd, clients);
	std::map<int, Client*> cgi_pipes;
	int connection_alive = 1;

	connection_alive = timeout_check(client, pfd.fd, pfds, clients);
	//TODO insert cgi timeout
	
	//HANDE CGI EOF
	if (pfd.revents & POLLHUP)
	{
		cleanup_cgi(pfds, pfd, client);
		connection_alive = 0;
		return connection_alive;
	}

	// READ
	if (pfd.revents & POLLIN)
	{

		if (!is_cgi_fd(pfd.fd, clients) && !handle_client_read(pfd.fd, pfd, client))
		{
			cleanup_client(pfd.fd, pfds, clients);
			connection_alive = 0;
			return connection_alive;
        	}
		if (client.is_cgi() && !client.is_cgi_running())
		{
			printf("TRIGGERS WHEN IT SHOULDNT\n");
			run_cgi("./www/cgi-bin/test.py", client, pfds, cgi_pipes);
                   	pfd.events = 0; //stop poollin pollout, im reading
        		client.set_cgi_running(1);
		}

		if (client.is_cgi_running() && handle_cgi_write(client.get_cgi_pipe() , client))
			{
				set_client_pollout(pfds, client);
				std::cout << "handle_write ok pid:"<<  client.get_cgi_pid() <<  std::endl; //needed?
				std::cout << "handle_write client fd:"<<  client.get_fd() <<  std::endl; //"	
			}
	}
	// WRITE
	else if (pfd.revents & POLLOUT && client.is_read_complete())
	{
		//exit(0);
		handle_client_write(client, server_config);
		if (client.is_write_complete())
		{
			if (client.get_keep_alive())
			{
				client.reset();
				pfd.events = POLLIN; // go back to reading
			}
			else
			{
				cleanup_client(pfd.fd, pfds, clients);
		    		connection_alive = 0;
				return connection_alive;
			}
		}
	}
	return connection_alive;
}


void    run_server(Server** servers, int server_count)
{
    std::vector<struct pollfd> pfds;
    std::map<int, Client> clients;
    size_t i = 0;

    add_server_sockets(servers, server_count, pfds);
    while (1)
    {
        if (ft_poll(pfds, 1000, clients) == -1) //TODO check which value instead of 1000
			break;
	i = 0;
        while (i < pfds.size())
        {
			//static int client_four = 0;
			//if (pfds[i].fd == 4)
		//		client_four++;
	//		if (client_four == 15)
//				exit(0);
			printf("\n\n___NEW PFD NOW____\n");
			printf("fd now is: '%d'\n---------------\n", pfds[i].fd);
			Server *server = is_server(pfds[i].fd, servers, server_count); //if fd is server, return server
			if (server) //if found, handle
				handle_server_fd(pfds[i], *server, pfds, clients);
			else if (!handle_client_fd(pfds[i], pfds, clients, *find_server_for_client(pfds[i].fd, clients, servers, server_count))) //if client has been cleaned up, skip iterator++
				continue ;
		i++;
		}
	}
	close_servers(servers, server_count);
}
