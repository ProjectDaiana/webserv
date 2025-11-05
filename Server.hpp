#ifndef SERVER_HPP
#define SERVER_HPP


#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <csignal>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "webserv.hpp"

typedef struct s_server         t_server;
typedef struct s_location       t_location;
typedef struct s_listen_binding t_listen_binding;
typedef struct s_data 			t_data;

struct                          s_location
{
    const char *path;              // URL path prefix (e.g. "/images")
    const char **accepted_methods; // e.g. "GET", "POST"
    int method_count;              // number of methods in the array
    const char *redirect;          // URL to redirect to (NULL if not)
    const char *root;              // filesystem root for this location
    int autoindex;                 // automatic directory listing enable flag
    const char                  *default_file;
    // if client requests directory instead of a specific file path,
    // this file is shown per default
    int upload_enabled;          // flag - 1 enabled, 0 disabled
    const char *upload_store;    // directory where uploads are stored
    int upload_count;       //how many uploads were made
    const char **cgi_extensions; // array of extensions that trigger cgi,
    //  can be .php and .phtml bc they handled by the sam einterpreter,
    //  we only need to handle one file extension (e.g. only .py or only .php) so we can also just have a const string here in theory
    int cgi_count; // number of extensions in the array
    const char                  *cgi_path;
    // the binary (interpreter) of the .php/.phtml files/whatever file extension we will choose
	const char					*cgi_upload_store;
};

struct                          s_listen_binding
{
    const char                  *host;
    int                         port;
};

struct                          s_server
{
    const char *name; // server name
    t_listen_binding            *lb;
    // pointer to array of listen bindings NOTE keeping it modular even if theres only one lb per server,
    //bc this way there can be a compare listen binding ft and not too much stuff has to be passed
    const char  * *error_pages; // array of file paths for error codes
    int error_page_count;       // amount of error pages
    int *error_codes;           // array of status codes (eg 404)
    int error_code_count;       // amount of error codes
    size_t max_bdy_size;                            
        // maximum allowed body size of the requests
    t_location **locations;          
        // pointer to locations array
    int location_count;        
        // length of that array
};



//NOTE this server only contains runtime data, any static data is stored by the config data in t_server struct


class Server {
	private:
		int _fd;
		t_server _config;
	public:
		Server() {};
		Server(t_data *d, t_server *config);
		~Server();
		// handle_finish_and_exit to avoid closing an fd before finishing writting
		int closeServer();
		
		int get_fd() const;
		sockaddr_in get_sockaddr();
		
		//TODO see if this function goes here or outside of the class
		static struct pollfd create_pollfd(int fd, short events, short revents);
		const t_server& get_config() const {return _config;}
};

#endif
