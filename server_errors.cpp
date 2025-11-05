/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_errors.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/04 00:55:51 by ltreser           #+#    #+#             */
/*   Updated: 2025/11/05 21:34:17 by darotche         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "errors.hpp"
#include <unistd.h>

void ft_exit(void)
{
	int status = 0;
	std::exit(status);
	//kill(getpid(), SIGTERM);
}


void	ft_error(t_arena *mem, const char *msg, int terminate)
{
	write(2, msg, ft_strlen(msg));
	if (mem && mem->used != 0)
		free_arena(mem);
	if (terminate)
		ft_exit();
}
