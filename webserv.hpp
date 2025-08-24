/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 17:26:54 by ltreser           #+#    #+#             */
/*   Updated: 2025/08/24 19:22:38 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#define PERM_MEM_SIZE 100
#define push_struct(type, arena) (type *)arena_alloc(arena, sizeof(type));

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct s_arena t_arena;
typedef struct s_data t_data;
typedef struct s_listen_binding t_listen_binding;
typedef struct s_server t_server;

struct s_server
{
	const char			*name; //server name
	t_listen_binding	*lb; //pointer to array of listen bindings
	int					lb_count; //length of that array
	const char			*error_pages; //array of file paths for error codes
	int					*error codes; //array of status codes (eg 404)
	size_t				max_bdy_size; //maximum allowed body size of the requests
	t_location			**locations; //pointer to locations array
	int					location_count; //length of that array
}

struct s_listen_binding
{
	char	*host;
	int		port;
};

struct s_arena
{
	uint8_t *start;
	size_t size;
	size_t used;
};


struct s_data
{
	t_arena *perm_memory;	
	//put more here
};


//memory
void    free_arena(t_arena *mem);
t_arena *alloc_perm_memory(void);
void *arena_alloc(t_arena *mem, size_t size);

//init
t_data 	*init_data(void);

//parser

//..



#endif
