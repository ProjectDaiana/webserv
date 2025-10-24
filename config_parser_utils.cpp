const t_token* parser_current(t_parser *p)
{
    return &p->lx->tokens[p->pos];
}

bool parser_advance(t_parser *p)
{
    if (p->pos < p->lx->token_count)
    {
        p->pos++;
        return true;
    }
    return false;
}

bool parser_match(t_parser *p, int type)
{
    if (parser_current(p)->type == type)
    {   
        parser_advance(p);
        return true;
    }
    return false;
}

