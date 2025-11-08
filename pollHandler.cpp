#include "pollHandler.hpp"
#include "CGI.hpp"
#include "runCGI.hpp"
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>

int validate_and_resolve_path(const t_server& config, Client& client) {
    std::string request_path = client.get_path(); // e.g. "/cgi-bin/test.py"

    t_location* location = find_location(request_path, config);
    if (!location) {
        client.set_error_code(404); // Not found
        return 404;
    }
	client.get_cgi().set_location(location); //will be used later for upload store
    
    size_t dot_pos = request_path.find_last_of(".");
    if (dot_pos == std::string::npos) {
        client.set_error_code(400); // Bad Request
        return 400;
    }
    
    std::string file_ext = request_path.substr(dot_pos);
    bool is_cgi_extension = false;
    for (int i = 0; i < location->cgi_count; i++) {
        if (file_ext == location->cgi_extensions[i]) {
            is_cgi_extension = true;
            break;
        }
    }
    
    if (!is_cgi_extension) {
        client.set_error_code(403); //Forbidden
        return 403;
    }

    std::string root(location->root);         // e.g. "www/html"
    std::string loc_path(location->path);     // e.g. "/cgi-bin" or "/"
    std::string document_root = root;
	std::string built_path = root + request_path; // e.g. "www/html/test.py" or "www/html/cgi-bin/test.py"
	
	// Remove trailing slash from document_root if present
	if (!document_root.empty() && document_root[document_root.size() - 1] == '/')
		document_root.erase(document_root.size() - 1);
	
    struct stat file_stat;
    if (stat(built_path.c_str(), &file_stat) != 0) {
        client.set_error_code(404);
        return 404;
    }

    if (!location->cgi_path) {
        client.set_error_code(500); //Internal Server Error
        return 500;
    }
    std::string interpreter = location->cgi_path;

    // Store resolved paths in the CGI object
    client.get_cgi().prepare_paths(
        built_path,         // SCRIPT_FILENAME
        client.get_path(),  // SCRIPT_NAME (original URI)
        document_root,      // DOCUMENT_ROOT
        interpreter         // interpreter
    );
    return 200;
}

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
	client.print_raw_request();
	std::cout << "DEBUG Host: " << client.get_header("Host") << std::endl;
	std::cout << "DEBUG Error: " << client.get_parse_error().code;
	std::cout << " " << client.get_parse_error().msg << std::endl;
	std::cout << "DEBUG Path: " << client.get_path() << std::endl;
	client.print_request_struct();
}

bool	handle_client_read(int fd, pollfd &pfd, Client &client, const t_server &server_config)
{
	char	buffer[256];
	int		bytes_read;

	client.update_activity();
	bytes_read = read(fd, buffer, sizeof(buffer));
	if (bytes_read <= 0)
	{
		if (!client.get_raw_request().empty())
		{
			if (!client.parse_request() || !client.is_headers_complete())
				client.set_error_code(400); // Bad Request
		}
		return (false);
	}
	client.add_to_request(buffer, bytes_read);
	
	// 413 Payload Too Large: Check if body exceeds max_body_size
	if (client.is_headers_complete() && server_config.max_bdy_size > 0) {
		size_t body_size = client.get_raw_request().size() - client.get_headers_end_pos();
		if (body_size > server_config.max_bdy_size) {
			client.set_error_code(413);
			client.set_read_complete(true);
			pfd.events = POLLOUT;
			return (true);
		}
	}
	
	if (client.is_read_complete())
	{
		if (client.get_error_code() == 200) {
			if (!client.parse_request()) // Calling parser, will also set_error
			{
				pfd.events = POLLOUT;
				return (true);
			}
		}
		//debug_request(client);
		if (!client.is_cgi()) 
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

int	find_pfd(int fd, std::vector<pollfd> &pfds)
{
    size_t i = 0;
    while (i < pfds.size())
    {
        if (pfds[i].fd == fd)
			return i;
        i++;
    }
	return (-1);
}

Client& find_client(int fd, std::map<int, Client> &clients)
{
    // First check if it's a normal client fd
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it != clients.end())
        return it->second;
    
    // If not found, check if it's a CGI fd
    int client_fd = find_client_for_cgi(fd, clients);
    if (client_fd != -1)
    {
        return clients.at(client_fd);
    }
    
    // This should not happen - throw or handle error
    return clients.at(fd); // This will throw if fd doesn't exist
}


void cleanup_client(int fd, std::vector<pollfd> &pfds, std::map<int, Client> &clients)
{
    Client &client = find_client(fd, clients);
    int client_fd = client.get_fd();

    // If client has CGI fds, remove/cleanup them first
    int stdout_fd = client.get_cgi_stdout_fd();
    int stdin_fd  = client.get_cgi_stdin_fd();

    if (stdout_fd != -1)
    {
        int pfd_index = find_pfd(stdout_fd, pfds);
        if (pfd_index != -1)
        {
            // call cleanup_cgi for proper handling (this will remove the stdout pfd and set client pollout)
            cleanup_cgi(pfds, pfds[pfd_index], client);
        }
    }

    // If stdin pipe still present in pfds, remove and close it
    if (stdin_fd != -1)
    {
        int pfd_index = find_pfd(stdin_fd, pfds);
        if (pfd_index != -1)
        {
			close(pfds[pfd_index].fd);
            pfds.erase(pfds.begin() + pfd_index);
        } else {
            // ensure fd closed if still valid
			// pfd_index == -1, just close the fd directly if it's valid
			if (stdin_fd >= 0)
				close(stdin_fd);
        }
    }

    // Close the client socket and erase the client entry by client fd
    close(client_fd);
    clients.erase(client_fd);

    // Remove client socket pollfd if still present
    int pfd_index = find_pfd(client_fd, pfds);
    if (pfd_index != -1)
        pfds.erase(pfds.begin() + pfd_index);
}

int ft_poll(std::vector<struct pollfd>& pfds, int timeout_ms, std::map<int, Client> &clients)
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
			if (now - i->second.get_last_activity() > CLIENT_INACTIVITY_TIMEOUT)
			{
				i->second.set_error_code(408); //Request Timeout
				i->second.set_read_complete(true);
				// Set client to POLLOUT to send error response
				int client_fd = i->first;
				int pfd_idx = find_pfd(client_fd, pfds);
				if (pfd_idx != -1)
					pfds[pfd_idx].events = POLLOUT;
				i++;
			}
			else
				i++;
		}

		// Also check for running CGI timeouts even if no poll events occurred.
		// If a CGI child is stuck (e.g. infinite loop with no stdout), poll won't report any event for its stdout fd,
		// so we need to explicitly enforce the CGI timeout here and arrange for the client to be polled for
		// writing the error response.
		std::map<int, Client>::iterator j = clients.begin();
		while (j != clients.end()) {
			Client &c = j->second;
			if (c.is_cgi_running()) {
				if (handle_cgi_timeout(c)) {				
					// Ensure the client socket is set to POLLOUT so the error response is sent
					int client_fd = j->first;
					int pfd_idx = find_pfd(client_fd, pfds);
					if (pfd_idx != -1)
						pfds[pfd_idx].events = POLLOUT;
					// If there is a CGI stdout fd still in pfds, clean it up (remove pipe fd and set client pollout)
					int stdout_fd = c.get_cgi_stdout_fd();
					int pfd_idx2 = -1;
					if (stdout_fd != -1)
						pfd_idx2 = find_pfd(stdout_fd, pfds);
					if (pfd_idx2 != -1) {
						cleanup_cgi(pfds, pfds[pfd_idx2], c);
					}
				}
			}
			j++;
		}/////
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
        if (i->second.get_cgi_stdout_fd() == cgi_fd)
			return i->first;
		else if (i->second.get_cgi_stdin_fd() == cgi_fd)
			return i->first;
        i++;
    }
    return -1;
}

int is_cgi_fd(int fd, const std::map<int, Client> &clients)
{
    if (clients.find(fd) != clients.end())
        return 0; // If fd is present as a client socket key, it's not a CGI pipe
    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.get_cgi_stdout_fd() == fd || it->second.get_cgi_stdin_fd() == fd)
            return 1; // is a CGI pipe fd
    }
    return -1; // unknown fd
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
	const int TIMEOUT = 30; // seconds
	if (now - client.get_last_activity() > TIMEOUT)
	{
		cleanup_client(fd, pfds, clients);
		return 0;
	}
	return 1;
}


void set_client_pollout(std::vector<pollfd> &pfds, Client &client)
{
	 int client_fd = client.get_fd();
         for (size_t i = 0; i < pfds.size(); ++i)
         {
			if (pfds[i].fd == client_fd)
			{
				pfds[i].events = POLLOUT;
				break;
			}
         }

}

void cleanup_cgi(std::vector<pollfd> &pfds, pollfd &pfd, Client &client)
{
	// cgi_eof already handles waitpid properly, so we just call it
	// Don't call waitpid here to avoid double-waiting
	cgi_eof(pfd.fd, client, pfds);
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
	Client &client = find_client(pfd.fd, clients);
	std::map<int, Client*> cgi_pipes;
	int connection_alive = 1;

	connection_alive = timeout_check(client, pfd.fd, pfds, clients);

	//HANDE CGI EOF
	if (pfd.revents & POLLHUP)
	{ 
	//	std::cout << "POLLHUP" << std::endl;
		cleanup_cgi(pfds, pfd, client);
		connection_alive = 0;
		return connection_alive;
	}

	// READ
	if (pfd.revents & POLLIN)
	{
		if (handle_cgi_timeout(client))
	    {
			if (client.is_write_complete()) {
				cleanup_cgi(pfds, pfd, client);
				connection_alive = 0;
			}
			pfd.events = POLLOUT;
   	    	return connection_alive;
   		}
		if (clients.find(pfd.fd) != clients.end() && !client.is_cgi_running() && !is_cgi_fd(pfd.fd, clients) && !handle_client_read(pfd.fd, pfd, client, server_config)) //if we dont wanna continue reading, cleanup client
		{
			cleanup_client(pfd.fd, pfds, clients);
			connection_alive = 0;
			return connection_alive;
        }
		if (client.is_cgi() && !client.is_cgi_running() && !handle_cgi_timeout(client))
		{
			if (client.get_error_code() != 200) {
				set_client_pollout(pfds, client);
				return connection_alive;
			}
			if (validate_and_resolve_path(server_config, client) != 200) {
				set_client_pollout(pfds, client);
				return connection_alive;
			}
			int cgi_error = run_cgi(client, pfds);
			if (cgi_error != 0) {
				client.set_error_code(cgi_error);
				set_client_pollout(pfds, client);
			}
			return connection_alive;
		}

		if (client.is_cgi_running() && handle_cgi_read_from_pipe(client.get_cgi_stdout_fd() , client,  pfds))
				set_client_pollout(pfds, client);
	}
	// WRITE
	else if (pfd.revents & POLLOUT && client.is_read_complete())
	{
		if (handle_cgi_timeout(client)) {
			pfd.events = POLLOUT;
			return connection_alive;
		}
		if (pfd.fd == client.get_cgi_stdin_fd() && client.is_cgi_writing()) {
			handle_cgi_write_to_pipe(pfd.fd, client, pfds);
			return connection_alive;
		}
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

void    run_server(Server** servers, int server_count, int idle_seconds, int poll_timeout_ms)
{
    std::vector<struct pollfd> pfds;
    std::map<int, Client> clients;
    size_t i = 0;

    add_server_sockets(servers, server_count, pfds);
	// track a server-level activity time so test-mode exit doesn't conflict
	// with per-client or CGI timeouts. We update this when any poll event
	// occurs (poll_result > 0). If test_idle_seconds > 0 and no activity has
	// occurred for that many seconds, exit the server loop.
	time_t server_last_activity = std::time(NULL);

	// Ensure poll timeout is reasonable (must be < smallest timeout we want to
	// enforce, e.g. CGI timeout) so ft_poll will wake up and check CGI timeouts.
	if (poll_timeout_ms <= 0)
		poll_timeout_ms = 1000;

	while (1)
	{
		int poll_result = ft_poll(pfds, poll_timeout_ms, clients);
		if (poll_result == -1)
			break;

		time_t now = std::time(NULL);
		if (poll_result > 0)
			server_last_activity = now; // activity -> reset server idle timer

		// If test-mode requested, exit if server has been idle for configured seconds
		if (idle_seconds > 0 && (now - server_last_activity) >= idle_seconds)
			break;
        i = 0;
        while (i < pfds.size())
        {
			Server *server = is_server(pfds[i].fd, servers, server_count); //if fd is server, return server
			if (server) //if found, handle
				handle_server_fd(pfds[i], *server, pfds, clients);
			else if (!handle_client_fd(pfds[i], pfds, clients, *find_server_for_client(pfds[i].fd, clients, servers, server_count))) //if client has been cleaned up, skip iterator++
				continue ;
		i++;
		}
	}
	write(1,"Exiting server gracefully\n", 27);
	close_servers(servers, server_count);
}
