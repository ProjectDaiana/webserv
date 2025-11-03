#include <cstring>   // for memcpy, strchr
#include <cstdlib>   // for atoi, NULL
#include <cstddef>   // for size_t
#include "webserv.hpp"


t_listen_binding  *parse_listen_binding(const char *str, t_arena *mem)
{
	printf("\033[31mLISTEN BINDING PARSER CALLED\033[0m\n");

	t_listen_binding *lb = create_listen_binding(mem);
	const char *colon = strchr(str, ':');
	if (colon)
	{	
		lb->host = arena_str(mem, str, (size_t)(colon - str));
		lb->port = atoi(colon + 1);
	}
	else
	{
		lb->port = atoi(str);
		lb->host = arena_str(mem, "127.0.0.1", 9);
	}	
	return (lb);
}

void parse_directive(t_parser *p, t_server *s, t_arena *mem, t_location *l) //if no l, l = null
{
	printf("\033[31mDIRECTIVE PARSER CALLED\033[0m\n");
	const char *name = parser_current(p)->value;
	parser_advance(p);
	if (parser_current(p)->type != TOK_STRING)
		//ft_error("Parser Error: expected value for directive!\n");
		;
	const char *value = parser_current(p)->value;
	parser_advance(p);
	printf("name: '%s', value: '%s'\n", name, value);
	if (strcmp(name, "allowed_methods") && !parser_match(p, TOK_SEMICOLON) )
		//ft_error("Parser Error: expected ';' after directive!\n");
		;
	if (l)
	{
		if (!strcmp(name, "root")) l->root = arena_str(mem, value);
		else if (!strcmp(name, "autoindex")) l->autoindex = (!strcmp(value, "on"));
		else if (!strcmp(name, "upload_store")) l->upload_store = arena_str(mem, value);
		else if (!strcmp(name, "upload_dir")) l->upload_store = arena_str(mem, value);
		else if (!strcmp(name, "index")) l->default_file = arena_str(mem, value);
		else if (!strcmp(name, "upload_enabled")) l->upload_enabled = (!strcmp(value, "on"));
		else if (!strcmp(name, "cgi_path")) l->cgi_path = arena_str(mem, value);
		else if (!strcmp(name, "cgi_upload_store")) l->cgi_upload_store = arena_str(mem, value);
		else if (!strcmp(name, "path")) l->path = arena_str(mem, value);
		else if (!strcmp(name, "redirect")) l->redirect = arena_str(mem, value);
		else if (!strcmp(name, "allowed_methods")) 
		{
			while (strcmp(value, ";"))
			{
				l->accepted_methods[l->method_count++] = arena_str(mem, value);
				value = parser_current(p)->value;
				parser_advance(p);
			}
		}		
		else if (!strcmp(name, "cgi_extensions"))
        {
            while (strcmp(value, ";"))
            {
                l->cgi_extensions[l->cgi_count++] = arena_str(mem, value);
                value = parser_current(p)->value;
                parser_advance(p);
            }
        }
		else
			//ft_error("Parser Error: unknown location directive!\n");
			;
	}
	else if (s)
	{
		//TODO integrate error pages
		if (!strcmp(name, "server_name")) s->name = arena_str(mem, value);
		else if (!strcmp(name, "max_body_size")) s->max_bdy_size = atoi(value);
		else if (!strcmp(name, "listen")) 
		{
			s->lb = parse_listen_binding(value, mem); 	
		}

	//	else
			//ft_error("Parser Error: unknown server directive!\n");
	}
	
}

t_location* parse_location(t_parser *p, t_arena *mem)
{
	printf("\033[31mLOCATION PARSER CALLED\033[0m\n");
	parser_advance(p); //skip 'location' token
	t_location *l = create_location(p, mem);
	if (parser_current(p)->type == TOK_STRING)
	{
		l->path = parser_current(p)->value;
		parser_advance(p);
	}
	if (!parser_match(p, TOK_LBRACE))
		//ft_error("Parser Error: expected '{' after *location path*!\n");
		;
	while (parser_current(p)->type != TOK_RBRACE && parser_current(p)->type != TOK_EOF)
		parse_directive(p, NULL, mem, l);
	parser_match(p, TOK_RBRACE);
	return (l);
}

t_server* parse_server(t_parser *p, t_arena *mem)
{
	printf("\033[31mSERVER PARSER CALLED\033[0m\n");
	t_server *s = create_server(p, mem);
	parser_advance(p); //skip 'server' token
	if (!parser_match(p, TOK_LBRACE))
		//ft_error("Parser Error: expected '{' after 'server'!\n");
		;
	while (parser_current(p)->type != TOK_RBRACE && parser_current(p)->type != TOK_EOF)
	{
		if (!strcmp(parser_current(p)->value, "location"))
			s->locations[s->location_count++] = parse_location(p, mem);
		else
			parse_directive(p, s, mem);
	}
	parser_match(p, TOK_RBRACE);
	return (s);
}

void	parser(t_data *d, t_parser *p, t_lexer *lx, t_arena *mem)
{
	init_parser(d, p, lx, mem);
	while (parser_current(p)->type != TOK_EOF)
	{
		if (!strcmp(parser_current(p)->value, "server"))
			d->s[d->server_count++] = parse_server(p, mem);
		else
		{
			//ft_error("Parser Error: unexpected token!\n");
			parser_advance(p);
		}
	}
}

