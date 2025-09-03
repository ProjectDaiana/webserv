/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 17:26:54 by ltreser           #+#    #+#             */
/*   Updated: 2025/09/04 00:10:18 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define PERM_MEM_SIZE 584 // TODO always update
# define push_struct(type, arena) (type *)arena_alloc(arena, sizeof(type));

# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>

typedef struct s_arena			t_arena;
typedef struct s_data			t_data;
typedef struct s_listen_binding	t_listen_binding;
typedef struct s_server			t_server;
typedef struct s_location		t_location;
typedef enum e_error			t_error;

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

	/* HTTP / request errors */
	ERR_BADREQ = 446,      /* Malformed HTTP request */
	ERR_URI_TOOLONG = 447, /* Request URI too long */
	ERR_METHOD = 448,      /* Unsupported HTTP method */

	/* Internal / general errors */
	ERR_UNKNOWN = 499 /* Unknown error */
};

struct							s_location
{
	const char *path;              // URL path prefix (e.g. "/images")
	const char **accepted_methods; // e.g. "GET", "POST"
	int method_count;              // number of methods in the array
	const char *redirect;          // URL to redirect to (NULL if not)
	const char *root;              // filesystem root for this location
	int autoindex;                 // automatic directory listing enable flag
	const char					*default_file;
	// if client requests directory instead of a specific file path,
	// this file is shown per default
	int upload_enabled;          // flag - 1 enabled, 0 disabled
	const char *upload_store;    // directory where uploads are stored
	const char **cgi_extensions; // array of extensions that trigger cgi,
	//	can be .php and .phtml bc they handled by the sam einterpreter,
	//	we only need to handle one file extension (e.g. only .py or only .php) so we can also just have a const string here in theory
	int cgi_count; // number of extensions in the array
	const char					*cgi_path;
	// the binary (interpreter) of the .php/.phtml files/whatever file extension we will choose
};

struct							s_server
{
	const char *name; // server name
	t_listen_binding			*lb;
	// pointer to array of listen bindings NOTE keeping it modular even if theres only one lb per server,
	bc this way there can be a compare listen binding ft and not too much stuff has to be passed const char
		* *error_pages; // array of file paths for error codes
	int error_page_count;                                                                                                  
		// amount of error pages
	int *error_codes;                                                                                                      
		// array of status codes (eg 404)
	int error_code_count;                                                                                                  
		// amount of error codes
	size_t max_bdy_size;                                                                                                   
		// maximum allowed body size of the requests
	t_location **locations;                                                                                                
		// pointer to locations array
	int location_count;                                                                                                    
		// length of that array
};

struct							s_listen_binding
{
	const char					*host;
	int							port;
};

struct							s_arena
{
	uint8_t						*start;
	size_t						size;
	size_t						used;
};

struct s_data // NOTE mb rename as config
{
	t_arena *perm_memory;
	t_server **s;
	int server_count;
};

// memory
void							free_arena(t_arena *mem);
t_arena							*alloc_perm_memory(void);
void							*arena_alloc(t_arena *mem, size_t size);

// init
t_data							*init_data(void);
void							init_config(t_data *d, t_arena *mem);

// parser

//..

#endif
