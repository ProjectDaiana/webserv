#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define PERM_MEM_SIZE 5000 // TODO always update
# define push_struct(type, arena) (type *)arena_alloc(arena, sizeof(type));

# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <map>
# include "Client.hpp"
# include "Server.hpp"

typedef struct s_arena			t_arena;
typedef struct s_data			t_data;
typedef struct s_listen_binding	t_listen_binding;
typedef struct s_server			t_server;
typedef struct s_location		t_location;
typedef struct s_request		t_request;
typedef struct s_response		t_response;
class Client;
class Server;

typedef enum e_error
{
 	/* No error */
 	ERR_NONE = 0, /* No error */

 	/* File / path errors */
 	ERR_NOENT = 434,  /* File not found */
 	ERR_PERM = 435,   /* Permission denied */
 	ERR_ISDIR = 436,  /* Expected a file but found a directory */
 	ERR_NOTDIR = 437, /* Expected a directory but found a file */

 	/* Memory / resource errors */
 	ERR_NOMEM = 438,  /* Out of memory */
 	ERR_MALLOC = 439, /* malloc failed */

 	/* Socket / network errors */
 	ERR_BIND = 440,    /* Bind failed: address already in use */
 	ERR_LISTEN = 441,  /* Listen failed */
 	ERR_ACCEPT = 442,  /* Accept failed */
 	ERR_CONNECT = 443, /* Connect failed */
 	ERR_SEND = 444,    /* send() failed */
 	ERR_RECV = 445,    /* recv() failed */
	ERR_AGAIN = 450,   /* Resource temporarily unavailable (non-blocking I/O) */

// 	/* HTTP / request errors */
	ERR_BADREQ = 446,      /* Malformed HTTP request */
	ERR_URI_TOOLONG = 447, /* Request URI too long */
	ERR_METHOD = 448,      /* Unsupported HTTP method */

 	/* Internal / general errors */
 	ERR_UNKNOWN = 499 /* Unknown error */
 } t_error;


struct							s_arena
{
	uint8_t						*start;
	size_t						size;
	size_t						used;
};

struct s_data 
{
	t_arena *perm_memory;
	t_server **s; //NOTE mb rename as config
	int server_count;
	Server **servers;
};

/*
//NOTE cut off uri before "?" in parser, ignore query str
struct	s_request
{
	std::string method; //can only be one ofc, for example GET
	std::string uri; //will be eg: "/cgi-bin/test.py", since we dont handle query str (at least for now)
	std::string path; 
    std::string query;
	std::string http_version; // eg: "HTTP/1.1", which version did the client use, impacts how we respond
	//TODO for which headers to implement -> check what each do and what we think makes sense to implement and what to leave out, also check subject if any headers are specifically required
	std::map<std::string, std::string> headers; //we should use a map here bc its easy to implement and use
	std::string body; //data the user is posting/putting into the website, for method post, so the body can be empty, depending on the request type
};*/

struct s_response
{
	//status line
	std::string version; //we might only implement 1 version, but its not rly much more work fyi
	int status_code; //from client, for example 200 (OK), 404 (not found)
	std::string reason_phrase; //e.g. "OK" or "Not found", from helper ft
	
	//headers
	std::string content_type; //MIME type = label that tells the client which type of content the body is
	size_t content_length; //length of the body
	std::string connection; //keep-alive or close. can be passed by the request, if not, dependant on http version if the default with no information is to keep-alive or close
	std::string location; //only for redirection, location of where to redirect to, by sending back the location of the redirection, the browser will automatically go to this location, for example for a moved page this can be true, or we could do a redirect to a nice song on youtube. the uri for"music" will then have the link "youtube.com/fesfbibesfcbslcsc"
	//body
	std::string body; //content of the http response
};

// memory
void							free_arena(t_arena *mem);
t_arena							*alloc_perm_memory(void);
void							*arena_alloc(t_arena *mem, size_t size);

// init
t_data							*init_data(void);
void							init_config(t_data *d, t_arena *mem);
void							init_servers(t_data *data);

// parser

//helper
int							ft_atoi(const char *nptr);
uint32_t						iptoi(const char *ip_str);

//polling
void handle_server_fd(pollfd &pfd, Server &server, std::vector<pollfd> &pfds, std::map<int, Client> &clients);
void    debug_request(Client &client);
bool    handle_client_read(int fd, pollfd &pfd, Client &client);
void    add_server_sockets(Server **servers, int server_count, std::vector<struct pollfd> &pfds);
int find_pfd(int fd, std::vector<pollfd> &pfds);
Client& find_client(int fd, std::map<int, Client> &clients);
void cleanup_cgi(std::vector<pollfd> &pfds, pollfd &pfd, Client &client);
void cleanup_client(int fd, std::vector<pollfd> &pfds, std::map<int, Client> &clients);
int ft_poll(std::vector<struct pollfd>& pfds, int timeout_ms, std::map<int, Client> &clients);
Server* is_server(int fd, Server** servers, int server_count);
void close_servers(Server **servers, int server_count);
int find_client_for_cgi(int cgi_fd, const std::map<int, Client> &clients);
int is_cgi_fd(int fd, const std::map<int, Client> &clients);
const t_server* find_server_for_client(int client_fd, const std::map<int, Client> &clients,
                                 Server **servers, int server_count);
int timeout_check(Client &client, int fd, std::vector<pollfd> &pfds, std::map<int, Client> &clients);
void set_client_pollout(std::vector<pollfd> &pfds, Client &client);
int handle_client_fd(pollfd &pfd, std::vector<pollfd> &pfds, std::map<int, Client> &clients, const t_server &server_config);
void    run_server(Server** servers, int server_count);


//response
std::string     handle_delete(Client &client, const t_server &config, t_location *l);
int     ft_delete(std::string path);
std::string handle_get(Client &client, const t_server &config, t_location *l);
std::string     file_to_str(Client &client, const std::string &path);
std::string autoindex_directory(Client &client, const std::string path);
std::string handle_post(Client &client, const t_server &config, t_location *l);
std::string     gen_filename(t_location *l);
int init_counter_from_dir(const std::string &upload_dir);
int extract_number(const char *name);
bool    method_allowed(const std::string& method, const t_location *location, Client &client);
t_location *find_location(std::string uri, const t_server &config);
std::string get_content_type(Client &client);
std::string get_reason_phrase(int code);
void    handle_client_write(Client &client, const t_server &config);
t_response      build_response(Client &client, const t_server &config);
std::string     handle_method(Client &client, const t_server &config, t_location *location);
std::string connection_type(Client &client);
std::string reload_page(Client &client);
t_location *handle_location(Client &client, const t_server &config);
std::string check_redirect(t_location *location, Client &client);

#endif
