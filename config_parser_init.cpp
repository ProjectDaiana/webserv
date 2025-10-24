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

t_server* create_server(t_data *d)
{
	t_server* s = (t_server *)arena_alloc(d->mem, sizeof(t_server));
    	s->name = "PumpkinServer";

    s->locations = nullptr;
    s->location_count = 0;
    s->lb = nullptr;
    s->error_pages = nullptr;
    s->error_codes = nullptr;
    s->error_page_count = 0;
    s->error_code_count = 0;
    s->max_bdy_size = 0;
    s->name = nullptr;
    return s;
}

t_location* create_location()
{
    t_location* loc = new t_location;
    loc->path = nullptr;
    loc->accepted_methods = nullptr;
    loc->method_count = 0;
    loc->redirect = nullptr;
    loc->root = nullptr;
    loc->autoindex = 0;
    loc->default_file = nullptr;
    loc->upload_enabled = 0;
    loc->upload_store = nullptr;
    loc->upload_count = 0;
    loc->cgi_extensions = nullptr;
    loc->cgi_count = 0;
    loc->cgi_path = nullptr;
    return loc;
}

