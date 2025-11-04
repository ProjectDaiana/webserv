#include "webserv.hpp"
#include "pollHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"

#include <stdio.h>
#include <iostream>


static void print_listen_binding(const t_listen_binding *lb)
{
    printf("  Listen:\n");
    printf("    Host: %s\n", lb->host ? lb->host : "(none)");
    printf("    Port: %d\n", lb->port);
}

static void print_error_pages(const t_server *s)
{
    printf("  Error pages (%d):\n", s->error_page_count);
    for (int i = 0; i < s->error_page_count; ++i)
        printf("    [%d] => %s\n", s->error_codes[i], s->error_pages[i]);
}


static void print_locations(const t_server *s)
{
    printf("  Locations (%d):\n", s->location_count);
    for (int i = 0; i < s->location_count; ++i)
    {
        const t_location *l = s->locations[i];
        printf("    %s\n", l->path ? l->path : "(unnamed)");

        printf("      Root: %s\n", l->root ? l->root : "(none)");
        printf("      Default file: %s\n", l->default_file ? l->default_file : "(none)");
        printf("      Autoindex: %s\n", l->autoindex ? "on" : "off");

        if (l->method_count > 0)
        {
            printf("      Allowed methods:");
            for (int j = 0; j < l->method_count; ++j)
                printf(" %s", l->accepted_methods[j]);
            printf("\n");
        }
        else
            printf("      Allowed methods: (none)\n");

        // Always print upload info if present, even if upload_enabled is off
        if (l->upload_store || l->upload_enabled)
        {
            printf("      Upload store: %s\n", l->upload_store ? l->upload_store : "(none)");
            printf("      Upload enabled: %s\n", l->upload_enabled ? "on" : "off");
        }

        if (l->cgi_count > 0)
        {
            printf("      CGI extensions:");
            for (int k = 0; k < l->cgi_count; ++k)
                printf(" %s", l->cgi_extensions[k]);
            printf("\n");
            printf("      CGI path: %s\n", l->cgi_path ? l->cgi_path : "(none)");
        }
    }
}

static void print_server(const t_server *s, int index)
{
    printf("Server [%d]: %s\n", index, s->name ? s->name : "(unnamed)");
    if (s->lb)
        print_listen_binding(s->lb);
    printf("  Max body size: %zu\n", s->max_bdy_size);
    print_error_pages(s);
    print_locations(s);
    printf("--------------------\n");
}

void print_data(const t_data *d)
{
    printf("=== CONFIG DATA ===\n");
    for (int i = 0; i < d->server_count; ++i)
        print_server(d->s[i], i);
}


struct ListenSnapshot {
    const void *lb_ptr;
    const void *host_ptr;
    int port;
    bool valid;
};
/*
void debug_compare_servers()
{
    static ListenSnapshot prev[128] = {};
    static int prev_count = 0;

    if (!global) 
	{
        std::cerr << "[DEBUG] global is null\n";
        return;
    }
    int count = global->server_count;
    std::cerr << "\n=== DEBUG LISTEN BINDINGS ===\n";
	if (!count && global->n)
		 printf("HI");
    for (int i = 0; i < count; ++i) {
        t_server *srv = global->s[i];
        if (!srv || !srv->lb) {
            std::cerr << "[DEBUG] server " << i << " has no lb\n";
            continue;
        }

        const void *lb_ptr   = srv->lb;
        const void *host_ptr = srv->lb->host;
        int port             = srv->lb->port;

        std::cerr << "Server[" << i << "]\n";
        std::cerr << "  current: lb=" << lb_ptr
                  << ", host=" << host_ptr
                  << ", port=" << port << "\n";

        if (prev[i].valid) {
            std::cerr << "  previous: lb=" << prev[i].lb_ptr
                      << ", host=" << prev[i].host_ptr
                      << ", port=" << prev[i].port << "\n";
        } else {
            std::cerr << "  previous: (none)\n";
        }

        std::cerr << "----------------------------------------\n";

        // store snapshot
        prev[i].lb_ptr   = lb_ptr;
        prev[i].host_ptr = host_ptr;
        prev[i].port     = port;
        prev[i].valid    = true;
    }

    prev_count = count;
    std::cerr << "========================================\n\n";
}*/

int	main(void)
{
	t_data *data = init_data();
	//global = data;
	init_config(data, data->perm_memory);
	print_data(data);
	//exit(0);
	init_servers(data);
	run_server(data->servers, data->server_count);	
	free_arena(data->perm_memory);
	return (0);
}

