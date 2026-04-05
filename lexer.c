#include "sp.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

void lex_all(Lexer *lexer, Token **head, Token **tail)
{
    Token *token = NULL;

    while ((token = lexer_next(lexer)) != NULL)
    {
        if (*head == NULL) 
        {
            *head = *tail = token;
        }
        else
        {
            (*tail)->next = token;
            *tail = token;
        }
    }
}

char *slice(char *begin, char *end)
{
    size_t len = end - begin;
    char *dest = malloc(len + 1);
    memcpy(dest, begin, len);
    dest[len] = 0;
    return dest;
}

Token *new_token(TokenKind kind, char *value)
{
    Token *token = malloc(sizeof(Token));
    token->kind = kind;
    token->value = value;
    return token;
}


void lexer_error(Lexer *lexer, char *efmt, ...)
{
    va_list valst;
    va_start(valst, efmt);

    fprintf(stderr, "[Lexer error at %s %zu:%zu] ",
            lexer->locus.file,
            lexer->locus.line,
            lexer->locus.col);
    vfprintf(stderr, efmt, valst); 

    va_end(valst);
}

const char *strtokkind(TokenKind kind)
{
    switch(kind)
    {
        case TK_LPAREN: return "(";
        case TK_RPAREN: return ")";
        case TK_NAME: return "name";
        case TK_NUMBER: return "number";
        case TK_STRING: return "string";
        case TK_TICK: return "tick";
        case TK_BACKTICK: return "backtick";
        case TK_COMMA: return "comma";
        case TK_COMMA_AT: return "comma-at";
    }
    return "unknown token";
}

void assign_locus(Token *token, Lexer *lexer)
{
    token->locus = lexer->locus;
}

Lexer *new_lexer(char *file, char *source)
{
    assert(source != NULL);

    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->locus.file = file;
    lexer->locus.line = 1;
    lexer->locus.col = 1;
    lexer->source = source;
    lexer->position = source;
    lexer->balance = 0;

    return lexer;
}

void lexer_advance(Lexer *lexer)
{
    char ch = *lexer->position;

    if (!ch) return;

    lexer->position++;

    if (ch == '\n')
    {
        lexer->locus.col = 1;
        lexer->locus.line++;
        return;
    }

    lexer->locus.col++;
}

int is_namechar(char ch)
{
    return !isspace(ch)
        && (ch != '(')
        && (ch != ')')
        && (ch != '\'');
}

int is_prefix(char *str, char *prefix)
{
    if (strlen(str) < strlen(prefix))
        return 0;

    return !strncmp(str, prefix, strlen(prefix));
}

Token *lex_number_b16(Lexer *lexer)
{
    char *begin = lexer->position;
    // Skip 0x prefix
    lexer_advance(lexer);
    lexer_advance(lexer);

    while (isxdigit(*lexer->position))
    {
        lexer_advance(lexer);
    }

    char *end = lexer->position;

    char *value = slice(begin, end);

    Token *token = new_token(TK_NUMBER, value);
    assign_locus(token, lexer);

    return token;
}

Token *lex_number_b8(Lexer *lexer)
{
    char *begin = lexer->position;
    // Skip 0o prefix
    lexer_advance(lexer);
    lexer_advance(lexer);

    while (is_odigit(*lexer->position))
    {
        lexer_advance(lexer);
    }

    char *end = lexer->position;

    char *value = slice(begin, end);

    Token *token = new_token(TK_NUMBER, value);
    assign_locus(token, lexer);

    return token;
}

Token *lex_number_b2(Lexer *lexer)
{
    char *begin = lexer->position;
    // Skip 0b prefix
    lexer_advance(lexer);
    lexer_advance(lexer);

    while (is_bdigit(*lexer->position))
    {
        lexer_advance(lexer);
    }

    char *end = lexer->position;

    char *value = slice(begin, end);

    Token *token = new_token(TK_NUMBER, value);
    assign_locus(token, lexer);

    return token;
}

Token *lex_number_b10(Lexer *lexer)
{
    size_t dots = 0;

    char *begin = lexer->position;

    while (isdigit(*lexer->position)
            || *lexer->position == '.')
    {
        if (*lexer->position == '.') dots++;

        if (dots > 1) break;

        lexer_advance(lexer);
    }

    char *end = lexer->position;

    char *value = slice(begin, end);

    Token *token = new_token(TK_NUMBER, value);
    assign_locus(token, lexer);

    return token;
}

Token *lex_number(Lexer *lexer)
{
    if (is_prefix(lexer->position, "0x"))
    {
        return lex_number_b16(lexer);
    }

    if (is_prefix(lexer->position, "0o"))
    {
        return lex_number_b8(lexer);
    }

    if (is_prefix(lexer->position, "0b"))
    {
        return lex_number_b2(lexer);
    }

    return lex_number_b10(lexer);
}

int is_bdigit(char chr)
{
    return chr == '1' || chr == '0';
}

int is_odigit(char chr)
{
    return isdigit(chr)
        && chr >= '0'
        && chr < '8';
}

Token *lex_string(Lexer *lexer)
{
    char *begin = lexer->position;

    int backslashed = 0;
    lexer_advance(lexer);
    for(char ch; (ch = *lexer->position); lexer_advance(lexer))
    {
        if (backslashed)
        {
            backslashed = 0;
            continue;
        }
        if (ch == '\\') backslashed = 1;
        else if (ch == '"') break;
    }

    char *end = lexer->position;

    if (*end != '"')
    {
        lexer_error(lexer, "EOF before end of string\n");
        exit(1);
    }

    lexer_advance(lexer);

    char *value = slice(begin, end + 1);

    Token *token = new_token(TK_STRING, value);
    assign_locus(token, lexer);

    return token;
}

Token *lex_name(Lexer *lexer)
{
    char *begin = lexer->position;

    while(is_namechar(*lexer->position)) lexer_advance(lexer);

    char *end = lexer->position;

    char *value = slice(begin, end);

    Token *token = new_token(TK_NAME, value);
    assign_locus(token, lexer);

    return token;
}

Token *lexer_next(Lexer *lexer)
{
    while (isspace(*lexer->position)) lexer_advance(lexer);

    if (!*lexer->position) return NULL;

    Token *token = NULL;
    switch(*lexer->position)
    {
        case '(':
            lexer_advance(lexer);
            token = new_token(TK_LPAREN, NULL);
            assign_locus(token, lexer);
            lexer->balance++;
            return token;
        case ')':
            lexer_advance(lexer);
            token = new_token(TK_RPAREN, NULL);
            assign_locus(token, lexer);
            lexer->balance--;
            return token;
        case '\'':
            lexer_advance(lexer);
            token = new_token(TK_TICK, NULL);
            assign_locus(token, lexer);
            return token;
        case '`':
            lexer_advance(lexer);
            token = new_token(TK_BACKTICK, NULL);
            assign_locus(token, lexer);
            return token;
        case ',':
            lexer_advance(lexer);
            token = new_token(TK_COMMA, NULL);
            assign_locus(token, lexer);
            if (*lexer->position == '@')
            {
                lexer_advance(lexer);
                token->kind = TK_COMMA_AT;
            }
            return token;
        case '"':
            return lex_string(lexer);
    }

    if (isdigit(*lexer->position)) return lex_number(lexer);

    if (is_namechar(*lexer->position)) return lex_name(lexer);

    lexer_error(lexer, "Invalid token '%c'\n", *lexer->position);
    exit(1);
}
