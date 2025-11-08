#include "webserv.hpp"

void    init_servers(t_data *d)
{
    int i;

    i = 0;
    d->servers = (Server **)arena_alloc(d->perm_memory, d->server_count * sizeof(Server));
	while(i < d->server_count)
    {
		d->servers[i] = (Server *)arena_alloc(d->perm_memory, sizeof(Server));
        new (d->servers[i]) Server(d, d->s[i]);
        i++;
    }
}

t_data *init_data(const char *config_path)
{
	t_arena *mem;
	t_data	*d;

	mem = alloc_perm_memory();
	d = (t_data *)arena_alloc(mem, sizeof(t_data));
	d->perm_memory = mem;
	d->config_path = config_path;
	d->server_count = 0;
	d->n = 0;
	return (d);
}

