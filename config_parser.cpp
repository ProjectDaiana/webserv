#include <cstdlib>   // for atoi, NULL
#include <cstddef>   // for size_t
#include "webserv.hpp"


t_listen_binding  *parse_listen_binding(const char *str, t_arena *mem)
{
//	printf("\033[31mLISTEN BINDING PARSER CALLED\033[0m\n");

	t_listen_binding *lb = create_listen_binding(mem);
	const char *colon = ft_strchr(str, ':');
	if (colon)
	{	
		lb->host = arena_str(mem, str, (size_t)(colon - str));
		lb->port = ft_atoi(colon + 1);
	}
	else
	{
		lb->port = ft_atoi(str);
		lb->host = arena_str(mem, "127.0.0.1", 9);
	}	
	return (lb);
}

void parse_directive(t_parser *p, t_server *s, t_arena *mem, t_location *l) //if no l, l = null
{
//	printf("\033[31mDIRECTIVE PARSER CALLED w '%s'\033[0m\n", parser_current(p)->value);
	const char *name = parser_current(p)->value;
	parser_advance(p);
	if (parser_current(p)->type != TOK_STRING && !ft_strcmp(name, "server"))
		ft_error(mem, BRACE_MISSING, 1);
	if (parser_current(p)->type != TOK_STRING)
		ft_error(mem, EXPECTED_VALUE, 1);
	const char *value = parser_current(p)->value;
	parser_advance(p);
	if (simple_directive(name) && !parser_match(p, TOK_SEMICOLON)) //expect ; if simple dir
	{
		ft_error(mem, EXPECTED_SEMICOLON, 1);
	}
	if (l)
	{
		if (!ft_strcmp(name, "root")) l->root = arena_str(mem, value);
		else if (!ft_strcmp(name, "autoindex")) l->autoindex = (!ft_strcmp(value, "on"));
		else if (!ft_strcmp(name, "upload_store")) l->upload_store = arena_str(mem, value);
		else if (!ft_strcmp(name, "upload_dir")) l->upload_store = arena_str(mem, value);
		else if (!ft_strcmp(name, "index")) l->default_file = arena_str(mem, value);
		else if (!ft_strcmp(name, "upload_enabled")) l->upload_enabled = (!ft_strcmp(value, "on"));
		else if (!ft_strcmp(name, "cgi_path")) l->cgi_path = arena_str(mem, value);
		else if (!ft_strcmp(name, "cgi_upload_store")) l->cgi_upload_store = arena_str(mem, value);
		else if (!ft_strcmp(name, "path")) l->path = arena_str(mem, value);
		else if (!ft_strcmp(name, "redirect")) l->redirect = arena_str(mem, value);
		else if (!ft_strcmp(name, "allowed_methods")) 
		{
			while (ft_strcmp(value, ";"))
			{
				l->accepted_methods[l->method_count++] = arena_str(mem, value);
				value = parser_current(p)->value;
				parser_advance(p);
			}
		}		
		else if (!ft_strcmp(name, "cgi_extensions"))
        {
			l->cgi_path = "/usr/bin/python3";
            while (ft_strcmp(value, ";"))
            {
                l->cgi_extensions[l->cgi_count++] = arena_str(mem, value);
                value = parser_current(p)->value;
                parser_advance(p);
            }
        }
		else
			ft_error(mem, UNKNOWN_LOCATION_DIRECTIVE, 1);
	}
	else if (s)
	{
		if (!ft_strcmp(name, "server_name")) s->name = arena_str(mem, value);
		else if (!ft_strcmp(name, "max_body_size")) s->max_bdy_size = ft_atoi(value);
		else if (!ft_strcmp(name, "listen")) 
		{
			s->lb = parse_listen_binding(value, mem); 	
		}
		else if (!ft_strcmp(name, "error_page"))
		{
			s->error_codes[s->error_code_count++] = ft_atoi(value);
			value = parser_current(p)->value;
			parser_advance(p);
			s->error_pages[s->error_page_count++] = arena_str(mem, value);
			if (!parser_match(p, TOK_SEMICOLON))
				ft_error(mem, EXPECTED_SEMICOLON, 1);
		}
		else
			ft_error(mem, UNKNOWN_SERVER_DIRECTIVE, 1);
	}
	
}

t_location* parse_location(t_parser *p, t_arena *mem)
{
//	printf("\033[31mLOCATION PARSER CALLED\033[0m\n");
	parser_advance(p); //skip 'location' token
	t_location *l = create_location(p, mem);
	if (parser_current(p)->type == TOK_STRING)
	{
		l->path = parser_current(p)->value;
		parser_advance(p);
	}
	if (!parser_match(p, TOK_LBRACE))
		ft_error(mem, EXPECTED_LOCATION_BRACE, 1);
	while (parser_current(p)->type != TOK_RBRACE && parser_current(p)->type != TOK_EOF)
		parse_directive(p, NULL, mem, l);
	parser_match(p, TOK_RBRACE);
	return (l);
}

t_server* parse_server(t_parser *p, t_arena *mem)
{
//	printf("\033[31mSERVER PARSER CALLED\033[0m\n");
	t_server *s = create_server(p, mem);
	parser_advance(p); //skip 'server' token
	if (!parser_match(p, TOK_LBRACE))
		ft_error(mem, EXPECTED_SERVER_BRACE, 1);
	while (parser_current(p)->type != TOK_RBRACE && parser_current(p)->type != TOK_EOF)
	{
		if (!ft_strcmp(parser_current(p)->value, "location"))
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
		if (!ft_strcmp(parser_current(p)->value, "server"))
			d->s[d->server_count++] = parse_server(p, mem);
		else
		{
			ft_error(mem, UNEXPECTED_TOKEN, 1);
			parser_advance(p);
		}
	}
}

