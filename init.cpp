/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:34:29 by ltreser           #+#    #+#             */
/*   Updated: 2025/08/30 20:53:23 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

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
	d->s = (t_server *)arena_alloc(mem, sizeof(t_server));
	d->l = (t_location *)arena_alloc(mem, sizeof(t_location));
	d->s->name = "PumpkinServer";
	d->s->lb_count = 1;
	d->s->lb = (t_listen_binding **)arena_alloc(mem, d->s->lb_count * sizeof(t_listen_binding));
	d->s->lb[0] = (t_listen_binding *)arena_alloc(mem, sizeof(t_listen_binding));
	d->s->lb[0]->host = "0.0.0.0";
	d->s->lb[0]->port = 8080;
	d->s->error_page_count = 1;
	d->s->error_pages = (const char **)arena_alloc(mem, d->s->error_page_count * sizeof(const char *));
	d->s->error_pages[0] = "/errors/404.html";
	d->s->error_code_count = 1;
	d->s->error_codes = (int *)arena_alloc(mem, d->s->error_code_count * sizeof(int));
	d->s->error_codes[0] = 404;
	d->s->max_bdy_size = 1 * 1024 * 1024; //1mb
	d->s->location_count = 1;
	d->s->locations = (t_location **)arena_alloc(mem, d->s->location_count * sizeof(t_location *));
	d->s->locations[0] = (t_location *)arena_alloc(mem, sizeof(t_location));
	d->s->locations[0]->path = "/images";
	d->s->locations[0]->method_count = 1;
	d->s->locations[0]->accepted_methods = (const char **)arena_alloc(mem, d->s->locations[0]->method_count * sizeof(const char *));
	d->s->locations[0]->accepted_methods[0] = "GET";
	d->s->locations[0]->redirect = "https://cataas.com/cat";
	d->s->locations[0]->root = "/var/www/html";
	d->s->locations[0]->autoindex = 1; //OJO enabled
	d->s->locations[0]->default_file = "index.html";
	d->s->locations[0]->upload_enabled = 0; //OJO disabled
	d->s->locations[0]->upload_store = "/var/www/html/uploads";
	d->s->locations[0]->cgi_count = 1;
	d->s->locations[0]->cgi_extensions = (const char **)arena_alloc(mem, d->s->locations[0]->cgi_count * sizeof (const char *));
	d->s->locations[0]->cgi_extensions[0] = ".py";
	d->s->locations[0]->cgi_path = "/usr/bin/python3";
}
