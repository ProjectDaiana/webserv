/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   memory.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: darotche <darotche@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 17:31:14 by ltreser           #+#    #+#             */
/*   Updated: 2025/11/05 22:30:51 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

//function to allocate memory within an areana
void *arena_alloc(t_arena *mem, size_t size)
{
	if (!size)
		ft_error(mem, MEMORY_CORRUPTION, 0);
	void *result;
	if (!mem || ((mem->used + size) > mem->size))
	{
		ft_error(mem, MALLOC_FAIL, 1);
		return (NULL);
	}
	result = mem->start + mem->used;
	mem->used += size;
	return (result);
}

void	free_arena(t_arena *mem)
{
	free(mem->start);
}

//initialization of memory struct for permanent lifespan memory
t_arena *alloc_perm_memory(void)
{
	uint8_t	*temp_buf;
	t_arena	*mem;

	temp_buf = (uint8_t*)malloc(PERM_MEM_SIZE);
	if (!temp_buf)
		return (NULL);
	memset(temp_buf, 0, PERM_MEM_SIZE);
	mem = (t_arena *)temp_buf;
	mem->start = temp_buf;
	mem->size = PERM_MEM_SIZE;
	mem->used = sizeof(t_arena);
	return (mem);
}

char *arena_str(t_arena *mem, const char *src, size_t len)
{
    if (!src)
        return NULL;
    if (!len)
        len = strlen(src);
    char *dst = (char *)arena_alloc(mem, len + 1); // +1 for null terminator
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

