int count_servers(t_lexer *lx)
{
    int count = 0;
    int i = 0;

    while (i < lx->token_count)
    {
        if (lx->tokens[i].type == TOK_STRING &&
            strcmp(lx->tokens[i].value, "server") == 0)
        {
            count++;
        }
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
        if (t->type == TOK_STRING && strcmp(t->value, "server") == 0)
            break; // stop at next server
        if (t->type == TOK_RBRACE)
            break; // end of this server block
        if (t->type == TOK_STRING && strcmp(t->value, "error_page") == 0)
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

        if (t->type == TOK_STRING && strcmp(t->value, "location") == 0)
            break; // stop at next location
        if (t->type == TOK_RBRACE)
            break; // end of this location block

        if (t->type == TOK_STRING && strcmp(t->value, "cgi_extensions") == 0)
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

    return count;
}

int count_locations(t_parser *p) 
{
    int saved_pos = p->pos;
    int count = 0;

    while (!match_token(p, TOK_RBRACE)) {
        if (match_token(p, TOK_STRING) && strcmp(peek_token(p)->value, "location") == 0) {
            count++;
        }
        advance(p);
    }

    p->pos = saved_pos;
    return count;
}

int count_allowed_methods(t_parser *p)
{
    int count = 0;
    int i = p->pos;
    while (i < p->lx->token_count)
    {
        t_token *t = &p->lx->tokens[i];
        if (t->type == TOK_STRING && strcmp(t->value, "location") == 0)
            break; // stop at next location
        if (t->type == TOK_RBRACE)
            break; // end of this location block
        if (t->type == TOK_STRING && strcmp(t->value, "allowed_methods") == 0)
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
        d->s = (t_server **)arena_alloc(mem, max_servers * sizeof(t_server));
}

t_server* create_server(t_data *d, t_parser *p)
{
	t_server* s = (t_server *)arena_alloc(d->mem, sizeof(t_server));
    s->name = "PumpkinServer"; //default name, do we wanna name them in the config file?
	s->lb = (t_listen_binding *)arena_alloc(mem, sizeof(t_listen_binding));
	s->lb->host = NULL;
	s->lb->port = 0;
	s->error_page_count = 0;
	s->error_pages = (const char **)arena_alloc(mem, count_error_pages(p) * sizeof(const char *));
	s->error_code_count = 0;
	s->error_codes = (int *)arena_alloc(mem, count_error_pages(p) * sizeof(int));
	s->max_bdy_size = 0;
	s->location_count = 0;
	s->locations = (t_location **)arena_alloc(mem, count_locations(p) * sizeof(t_location *));
    return s;
}

t_location* create_location(t_parser *p)
{
	t_location *l = (t_location *)arena_alloc(mem, sizeof(t_location));
	l->path = NULL;
	l->method_count = 0;
	l->accepted_methods = (const char **)arena_alloc(mem, count_allowed_methods(p) * sizeof(const char *));
	l->redirect = NULL;
	l->root = NULL;
	l->autoindex = 0;
	l->default_file = NULL;
	l->upload_enabled = 0;
	l->upload_store = NULL;
	l->upload_count = 0; //only variable thats not parsed
	l->cgi_count = 0;
	l->cgi_extensions = (const char **)arena_alloc(mem, cgi_count_extensions(p) * sizeof (const char *));
	l->cgi_path = NULL;
}
