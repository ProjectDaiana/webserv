/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:34:29 by ltreser           #+#    #+#             */
/*   Updated: 2025/08/29 16:09:57 by ltreser          ###   ########.fr       */
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
void	init_config(t_data *d, t_arena *mem)
{
	d->s = (t_server *)arena_alloc(mem, sizeof(t_server));
	d->l = (t_location *)arena_alloc(mem, sizeof(t_location));
	d->s->name = (char *)arena_alloc(mem, 11);
	strcpy(d->s->name, "miniserver\n");
	d->s->lb_count = 1;
	d->s->lb = (t_listen_binding **)arena_alloc(mem, d->s->lb_count * sizeof(t_listen_binding);
	d->s->lb[0] = (t_listen_binding *)arena_alloc(mem, sizeof(t_listen_binding));
	d->s->lb[0]->host = (char *)arena_alloc(mem, 8);
	strcpy(d->s->lb[0]->host, "0.0.0.0\n");
	d->s->lb[0]->port = 8080;
	d->s->error_pages_count = 1;
	d->s->error_pages = (char **)arena_alloc(mem, d->s->error_pages_count * sizeof(char *));
	d->s->error_pages[0] = (char *)arena_alloc(mem, strlen("/errors/404.html" + 1);
	strcpy(d->s->error_pages[0], "/errors/404.html");
	d->s->error_codes_count = 1;
	d->s->error_codes = (int *)arena_alloc(mem, d->s->error_codes_count * 4);
	d->s->error_codes[0] = 404;
	d->s->max_bdy_size = 1 * 1024 * 1024; //1mb
	d->s->location_count = 1;
	d->s->locations = (t_location **)arena_alloc(mem, d->s->location_count * sizeof(t_location));
	d->s->locations[0] = (t_location *)arena_alloc(mem, sizeof(t_location));
	d->s->locations[0]->path = (char *)arena_alloc(mem, 8);
	stcpy(d->s->locations[0]->path, "/images");
	//TODO use str literals instead, bc read only makes mroe sense

		

}
