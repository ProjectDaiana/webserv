#include "webserv.hpp"

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

bool simple_directive(const char *str)
{
	if (ft_strcmp(str, "allowed_methods") && ft_strcmp(str, "cgi_extensions") && ft_strcmp(str, "error_page"))
		return true;
	else
		return false;
}
