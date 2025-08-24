/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 17:26:54 by ltreser           #+#    #+#             */
/*   Updated: 2025/08/24 16:50:05 by ltreser          ###   ########.fr       */
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
