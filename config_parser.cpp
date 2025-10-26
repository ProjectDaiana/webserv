
t_listen_binding  *parse_listen_binding(t_parser *p)
{
	t_listen_binding *lb;
	const char *token = parser_current(p).value;
	if (token != TOK_STRING)
	{
		//ft_error("Parser Error: expected listen binding value!\n");
		return NULL;
	}
	const char *colon = strchr(token, ":");
	if (colon)
	{	
		memcpy(lb->host, token, (size_t)colon - token); //cpp style, working?
		lb->port = atoi(colon + 1);
	}
	else
		lb->port = atoi(token);
	return (lb);
}

void parse_directive(t_parser *p, t_server *s, t_location *l = NULL) //if no l, l = null
{
	char *name = parser_current(p).value;
	parser_advance(p);
	if (parser_current(p).type == TOK_STRING)
	{
		char *value = parser_current(p).value;
		parser_advance(p);
	}
	if (!parser_match(p, TOK_SEMICOLON))
		//ft_error("Parser Error: expected ';' after directive!\n");
		;
	if (l)
	{
		if (!strcmp(name, "root")) l->root = value;
		else if (!strcmp(name, "autoindex")) l->autoindex = (!strcmp(value, "on"))
		else if (!strcmp(name, "upload_store")) l->upload_store = value;
		else if (!strcmp(name, "upload_enabled")) l->upload_enabled = (!strcmp(value, "on"))
		else if (!strcmp(name, "cgi_path")) l->cgi_path = value;
		else if (!strcmp(name, "path")) l->path = value;
		else
			//ft_error("Parser Error: unknown location directive!\n");
			;
	}
	else if (s)
	{
		if (!strcmp(name, "listen")) s->lb = parse_listen_binding(p);
		else if (!strcmp(name, "server_name")) s->name = value;
		else
			//ft_error("Parser Error: unknown server directive!\n");
	}
	//TODO insert safety net for in case parser didnt advance?
}

t_location* parse_location(t_parser *p)
{
	t_location *l = create_location();
	parser_advance(p); //skip 'location' token
	if (parser_current(p).type == TOK_STRING)
	{
		l->path = parser_current(p).value;
		parser_advance(p);
	}
	if (!parser_match(TOK_LBRACE))
		//ft_error("Parser Error: expected '{' after *location path*!\n");
		;
	while (parser_current(p).type != TOK_RBRACE && parser_current(p).type != TOK_EOF)
		parse_directive(p, NULL, l);
	parser_match(p, TOK_RBRACE);
	return (l);
}

t_server* parse_server(t_parser *p)
{
	t_server *s = create_server();
	parser_advance(p); //skip 'server' token
	if (!parser_match(p, TOK_LBRACE))
		//ft_error("Parser Error: expected '{' after 'server'!\n");
		;
	while (parser_current(p).type != TOK_RBRACE && parser_current(p).type != TOK_EOF)
	{
		if (!strcmp(parser_current(p).value, "location"))
			s->locations[s->location_count++] = parse_location(p);
		else
			parse_directive(p, s);
	}
	parser_match(p, TOK_RBRACE);
	return (s);
}

void	parser(t_data *d, t_parser *p, t_lexer *lx, t_arena *mem)
{
	init_parser(d, p, lx, mem);
	while (parser_current(p).type != TOK_EOF)
	{
		if (!strcmp(parser_current(p).value, "server"))
			d->s[d->server_count++] = parse_server(p);
		else
		{
			//ft_error("Parser Error: unexpected token!\n");
			parser_advance(p);
		}
	}
}

//TODO pass memory everywhere

