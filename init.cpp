/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:34:29 by ltreser           #+#    #+#             */
/*   Updated: 2025/09/27 02:00:25 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

void    init_servers(t_data *data)
{
    int i;

    i = 0;
    while(i < data->server_count)
    {
        new (data->servers[i]) Server(data->s[i]);
        i++;
    }
}

t_data *init_data(void)
{
	t_arena *mem;
	t_data	*d;

	mem = alloc_perm_memory();
	d = (t_data *)arena_alloc(mem, sizeof(t_data));
	d->perm_memory = mem;
	init_config(d, d->perm_memory);
	//init rest here
	return (d);
}

//allocating and hardcoding config for listen_binding, server, location
//OJO only string literals, cant be changed later, but dont have to be
//chunky monkey, make smaller once hardcoding is deleted
void	init_config(t_data *d, t_arena *mem)
{
	d->server_count = 1;
	d->s = (t_server **)arena_alloc(mem, d->server_count * sizeof(t_server));
	d->servers = (Server **)arena_alloc(mem, d->server_count * sizeof(Server));
	d->servers[0] = (Server *)arena_alloc(mem, sizeof(Server));
	d->s[0] = (t_server *)arena_alloc(mem, sizeof(t_server));
	d->s[0]->name = "PumpkinServer";
	d->s[0]->lb = (t_listen_binding *)arena_alloc(mem, sizeof(t_listen_binding));
	d->s[0]->lb->host = "0.0.0.0";
	d->s[0]->lb->port = 8080;
	d->s[0]->error_page_count = 1;
	d->s[0]->error_pages = (const char **)arena_alloc(mem, d->s[0]->error_page_count * sizeof(const char *));
	d->s[0]->error_pages[0] = "/errors/404.html";
	d->s[0]->error_code_count = 1;
	d->s[0]->error_codes = (int *)arena_alloc(mem, d->s[0]->error_code_count * sizeof(int));
	d->s[0]->error_codes[0] = 404;
	d->s[0]->max_bdy_size = 1 * 1024 * 1024; //1mb
	d->s[0]->location_count = 1;
	d->s[0]->locations = (t_location **)arena_alloc(mem, d->s[0]->location_count * sizeof(t_location *));
	d->s[0]->locations[0] = (t_location *)arena_alloc(mem, sizeof(t_location));
	d->s[0]->locations[0]->path = "/";
	d->s[0]->locations[0]->method_count = 3;
	d->s[0]->locations[0]->accepted_methods = (const char **)arena_alloc(mem, d->s[0]->locations[0]->method_count * sizeof(const char *));
	d->s[0]->locations[0]->accepted_methods[0] = "GET";
	d->s[0]->locations[0]->accepted_methods[1] = "POST";
	d->s[0]->locations[0]->accepted_methods[2] = "DELETE";
	d->s[0]->locations[0]->redirect = "https://cataas.com/cat";
	d->s[0]->locations[0]->root = "www/html";
	d->s[0]->locations[0]->autoindex = 0; //OJO disabled
	d->s[0]->locations[0]->default_file = "index.html";
	d->s[0]->locations[0]->upload_enabled = 1; //OJO enabled
	d->s[0]->locations[0]->upload_store = "www/html/uploads";
	d->s[0]->locations[0]->upload_count = 0;
	d->s[0]->locations[0]->cgi_count = 1;
	d->s[0]->locations[0]->cgi_extensions = (const char **)arena_alloc(mem, d->s[0]->locations[0]->cgi_count * sizeof (const char *));
	d->s[0]->locations[0]->cgi_extensions[0] = ".py";
	d->s[0]->locations[0]->cgi_path = "/usr/bin/python3";
}
