/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: darotche <darotche@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:26:00 by ltreser           #+#    #+#             */
/*   Updated: 2025/09/03 16:26:25 by darotche         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "pollHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"


// main parser
// int	main(void)
// {
// 	t_data *data = init_data();
// 	printf("data allocated\n");
// 	init_data();
// 	free_arena(data->perm_memory);
// 	printf("data freed\n");
// 	return (0);
// }


int	main(void)
{
	t_data *data = init_data();
	init_config(data, data->perm_memory); //hardcoded parser
	Server server(data->s[0]); //later this should be in a seperate ft, and looped
	run_server(server);	
	free_arena(data->perm_memory);
	return (0);
}

