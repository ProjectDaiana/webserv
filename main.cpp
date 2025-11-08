#include "webserv.hpp"
#include "pollHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"

#include <stdio.h>
#include <iostream>


int	main(int ac, char **av)
{
	if (2 != ac)
		ft_error(NULL, TOO_FEW_ARGS, 1);
	t_data *data = init_data(av[1]);
	init_config(data, data->perm_memory);
	init_servers(data);
	run_server(data->servers, data->server_count);	
	free_arena(data->perm_memory);
	return (0);
}

