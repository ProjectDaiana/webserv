/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/04 00:55:51 by ltreser           #+#    #+#             */
/*   Updated: 2025/09/04 01:14:25 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <stdio.h>

//TODO change printf w write and write to stderror
int	ft_error(t_data *data, t_error error)
{
	printf("An error occured: %d", error);
	//close all fds NOTE: lets have an array that stores all open fds?
	if (!data)
		return (error);
	free_arena(data->perm_memory);
	return (error);
	//exit(0);
}
