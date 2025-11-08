#include "webserv.hpp"   // your structs: s_token, t_arena, t_lexer, enums, etc.

#include <cstddef>       // size_t
#include <cstdlib>       // NULL, TODO exit() DEL
#include <iostream>      // std::cout, std::cerr, std::string
#include <string>        // std::string
#include <cctype>        // isspace() TODO DEL LATER
#include <cstring>       // memcpy, TODO DEL LATER


static char lexer_peek(t_lexer *lx)
{
    return lx->input[lx->pos];
}

static char lexer_advance(t_lexer *lx)
{
    char c = lx->input[lx->pos++];
    if (c == '\n') 
    {
        lx->line++;
        lx->col = 1;
    } else {
        lx->col++;
    }
    return c;
}

static void lexer_skip_whitespace(t_lexer *lx)
{
    while (true)
    {
        char c = lexer_peek(lx);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            lexer_advance(lx);
        else
            break;
    }
}

static void lexer_init(t_lexer *lx, const std::string &config_content, t_arena *mem)
{
	lx->input = config_content.c_str();
	lx->pos = 0;
	lx->line = 1;
	lx->col = 1;
	lx->tokens = (t_token *)arena_alloc(mem, sizeof(t_token) * 2096);
	lx->token_count = 0;
	lx->capacity = 2096;
}

s_token lexer_next_token(t_lexer *lx, t_arena *mem)
{
    lexer_skip_whitespace(lx);
    s_token tok;
    tok.line = lx->line;
    tok.col = lx->col;
    char c = lexer_peek(lx);
    if (c == '\0') 
    {
        tok.type = TOK_EOF;
	tok.value = "EOF";
        return tok;
    }
    if (c == '{') { lexer_advance(lx); tok.value = "{"; tok.type = TOK_LBRACE; return tok; }
    if (c == '}') { lexer_advance(lx); tok.value = "}"; tok.type = TOK_RBRACE; return tok; }
    if (c == ';') { lexer_advance(lx); tok.value = ";"; tok.type = TOK_SEMICOLON; return tok; }
    std::string val;
    while (c && !ft_isspace(c) && c != '{' && c != '}' && c != ';')
    {
        val += lexer_advance(lx);
        c = lexer_peek(lx);
    }
    tok.type = TOK_STRING;
    tok.value = (char *)arena_alloc(mem, val.size() + 1);
    ft_memcpy((void *)tok.value, val.c_str(), val.size());
    return tok;
}

#include <iostream>
#include <iomanip> // for std::setw

void lexer_pretty_print(const t_lexer *lx)
{
    std::cout << std::left
              << std::setw(12) << "Type"
              << std::setw(20) << "Value"
              << std::setw(8)  << "Line"
              << std::setw(8)  << "Col"
              << "\n";

    std::cout << std::string(48, '-') << "\n";

    for (int i = 0; i < lx->token_count; ++i)
    {
        const s_token &tok = lx->tokens[i];
        std::string type_str;
        switch (tok.type)
        {
            case TOK_EOF:        type_str = "EOF"; break;
            case TOK_STRING:     type_str = "STRING"; break;
            case TOK_LBRACE:     type_str = "LBRACE"; break;
            case TOK_RBRACE:     type_str = "RBRACE"; break;
            case TOK_SEMICOLON:  type_str = "SEMICOLON"; break;
            default:             type_str = "UNKNOWN"; break;
        }

        std::cout << std::left
                  << std::setw(12) << type_str
                  << std::setw(20) << (tok.value ? tok.value : "(none)")
                  << std::setw(8)  << tok.line
                  << std::setw(8)  << tok.col
                  << "\n";
    }
}


void lexer(t_lexer *lx, const std::string &config_content, t_arena *mem)
{
	lexer_init(lx, config_content, mem);
	while (true)
	{
		t_token tok = lexer_next_token(lx, mem);
		lx->tokens[lx->token_count++] = tok;
		if (tok.type == TOK_EOF)
			break;
	}
	//lexer_pretty_print(lx);
}
