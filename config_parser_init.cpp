#include "webserv.hpp"
#include <string.h>           // for strcmp()
#include <stdlib.h>           // for atoi()


int count_servers(t_lexer *lx)
{
    int count = 0;
    int i = 0;

    while (i < lx->token_count)
    {
        if (lx->tokens[i].type == TOK_STRING &&
            ft_strcmp(lx->tokens[i].value, "server") == 0)
        	    count++;
        i++;
    }
    return count;
}

int count_error_pages(t_parser *p)
{
    int count = 0;
    int i = p->pos;
	while (i < p->lx->token_count)
    {
        t_token *t = &p->lx->tokens[i];
        if (t->type == TOK_STRING && ft_strcmp(t->value, "server") == 0)
		{
			i++;
            break; // stop at next server
		}
		i++;
    }
    while (i < p->lx->token_count)
    {
        t_token *t = &p->lx->tokens[i];
        if (t->type == TOK_STRING && ft_strcmp(t->value, "server") == 0)
            break; // stop at next server
        if (t->type == TOK_RBRACE)
            break; // end of this server block
        if (t->type == TOK_STRING && ft_strcmp(t->value, "error_page") == 0)
            count++;
        i++;
    }
    return count;
}

int count_cgi_extensions(t_parser *p)
{
    int count = 0;
    int i = p->pos;

    while (i < p->lx->token_count)
    {
        t_token *t = &p->lx->tokens[i];

        if (t->type == TOK_STRING && ft_strcmp(t->value, "location") == 0)
            break; // stop at next location
        if (t->type == TOK_RBRACE)
            break; // end of this location block

        if (t->type == TOK_STRING && ft_strcmp(t->value, "cgi_extensions") == 0)
        {
            // count extensions until the next semicolon
            i++;
            while (i < p->lx->token_count &&
                   p->lx->tokens[i].type != TOK_SEMICOLON)
            {
                if (p->lx->tokens[i].type == TOK_STRING)
                    count++;
                i++;
            }
        }
        i++;
    }
	printf(">count cgi extensions: %d\n", count);
    return count;
}

int count_locations(t_parser *p)
{
    int saved_pos = p->pos;
    int count = 0;
    int brace_depth = 0;

    while (p->pos < p->lx->token_count)
    {
        int type = parser_current(p)->type;
        const char *value = parser_current(p)->value;


        if (type == TOK_LBRACE)
            brace_depth++;
        else if (type == TOK_RBRACE) {
            brace_depth--;
            if (brace_depth == 0)
                break;  // End of server block
        }
        else if (type == TOK_STRING && brace_depth == 1 &&
                 ft_strcmp(value, "location") == 0) {
            count++;
        }
        parser_advance(p);
    }

    p->pos = saved_pos;
	printf(">location count is: %d\n", count);
    return count;
}

int count_allowed_methods(t_parser *p)
{
    int count = 0;
    int i = p->pos;
    while (i < p->lx->token_count)
    {
        t_token *t = &p->lx->tokens[i];
        if (t->type == TOK_STRING && ft_strcmp(t->value, "location") == 0)
            break; // stop at next location
        if (t->type == TOK_RBRACE)
            break; // end of this location block
        if (t->type == TOK_STRING && ft_strcmp(t->value, "allowed_methods") == 0)
        {
            // Count how many method tokens follow, until we hit a semicolon
            i++;
            while (i < p->lx->token_count &&
                   p->lx->tokens[i].type != TOK_SEMICOLON)
            {
                if (p->lx->tokens[i].type == TOK_STRING)
                    count++;
                i++;
            }
        }
        i++;
    }
    return count;
}


void    init_parser(t_data *d, t_parser *p, t_lexer *lx, t_arena *mem)
{
        int max_servers;
        p->lx = lx;
        p->pos = 0;
        p->mem = mem;
        d->server_count = 0;
        max_servers = count_servers(lx);
		if (count_servers(lx))
        	d->s = (t_server **)arena_alloc(mem, max_servers * sizeof(t_server *));
		//else
			//TODO throw error
}

t_listen_binding *create_listen_binding(t_arena *mem)
{
	t_listen_binding *lb = (t_listen_binding *)arena_alloc(mem, sizeof(t_listen_binding));
    lb->port = 0;
	lb->host = NULL;
	return lb;
}

t_server* create_server(t_parser *p, t_arena *mem)
{
	t_server* s = (t_server *)arena_alloc(mem, sizeof(t_server));
    s->name = "PumpkinServer"; //default name, do we wanna name them in the config file?
	s->error_page_count = 0;
	if (count_error_pages(p))
	{
		s->error_pages = (const char **)arena_alloc(mem, count_error_pages(p) * sizeof(const char *));
		s->error_codes = (int *)arena_alloc(mem, count_error_pages(p) * sizeof(int));
	}
	else
	{
		s->error_pages = NULL;
		s->error_codes = NULL;
	}
	s->error_code_count = 0;
	s->max_bdy_size = 0;
	s->location_count = 0;
	if (count_locations(p))
		s->locations = (t_location **)arena_alloc(mem, count_locations(p) * sizeof(t_location *));
	else
		s->locations = NULL;
    return s;
}

t_location* create_location(t_parser *p, t_arena *mem)
{
	t_location *l = (t_location *)arena_alloc(mem, sizeof(t_location));
	l->path = NULL;
	l->method_count = 0;
	if (count_allowed_methods(p))
		l->accepted_methods = (const char **)arena_alloc(mem, count_allowed_methods(p) * sizeof(const char *));
	else
		l->accepted_methods = NULL;
	l->redirect = NULL;
	l->autoindex = 0;
	l->default_file = NULL;
	l->upload_enabled = 0;
	l->upload_store = NULL;
	l->upload_count = 0; //only variable thats not parsed
	l->cgi_count = 0;
	if (count_cgi_extensions(p))
		l->cgi_extensions = (const char **)arena_alloc(mem, count_cgi_extensions(p) * sizeof (const char *));
	else
		l->cgi_extensions = NULL;
	l->cgi_path = NULL;
	return l;
}
