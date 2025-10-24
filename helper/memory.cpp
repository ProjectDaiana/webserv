/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   memory.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: darotche <darotche@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 17:31:14 by ltreser           #+#    #+#             */
/*   Updated: 2025/09/03 16:35:38 by darotche         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

//function to allocate memory within an areana
void *arena_alloc(t_arena *mem, size_t size)
{
	void *result;
	if (!mem || ((mem->used + size) > mem->size))
	{
		printf("ERROR: allocation fail\n");
		return (NULL);
	}
	result = mem->start + mem->used;
	mem->used += size;
	return (result);
}

void	free_arena(t_arena *mem)
{
	free(mem->start);
	mem->start = NULL;
}

//initialization of memory struct for permanent lifespan memory
t_arena *alloc_perm_memory(void)
{
	uint8_t	*temp_buf;
	t_arena	*mem;

	temp_buf = (uint8_t*)malloc(PERM_MEM_SIZE);
	if (!temp_buf)
		return (NULL);
	mem = (t_arena *)temp_buf;
	mem->start = temp_buf;
	mem->size = PERM_MEM_SIZE;
	mem->used = sizeof(t_arena);
	return (mem);
}


