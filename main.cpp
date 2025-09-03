/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:26:00 by ltreser           #+#    #+#             */
/*   Updated: 2025/09/01 18:52:05 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "pollHandler.hpp"
#include "server.hpp"
#include "client.hpp"

int	main(void)
{
	t_data *data = init_data();
	init_config(data, data->perm_memory); //hardcoded parser
	Server server(data->s[0]); //later this should be in a seperate ft, and looped
	run_server(server);	
	free_arena(data->perm_memory);
	return (0);
}

