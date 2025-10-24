
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
	init_servers(data);
	run_server(data->servers, data->server_count);	
	free_arena(data->perm_memory);
	return (0);
}

