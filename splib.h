/*
 * SPLIB -- S-expressions Parsing LIBrary.
 *
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */


#ifndef _SPLIB_H
#define _SPLIB_H

#include <stdlib.h>

typedef struct Lexer Lexer;
typedef struct Parser Parser;

typedef struct Locus Locus;
typedef struct Token Token;
typedef struct AST AST;

struct Locus
{
    char *file;
    size_t line, col;
};

enum TokenKind
{
    // (
    TK_LPAREN,
    // )
    TK_RPAREN,

    // "string"
    TK_STRING,
    // 102034, 0x0100fFaB, 0o070010354, 0b101001
    TK_NUMBER,
    // name
    TK_NAME,

    // '
    TK_TICK,
    // `
    TK_BACKTICK,
    // ,
    TK_COMMA,
    // ,@
    TK_COMMA_AT,
};
typedef enum TokenKind TokenKind;

struct Token
{
    TokenKind kind;
    Locus locus;
    char *value;
    Token *next;
};

struct Lexer
{
    ssize_t balance;
    Locus locus;
    char *source;
    char *position;
};

enum ASTKind
{
    // The root tree
    AK_ROOT,
    // Name, string, number
    AK_VALUE,
    // (<e1> <e2> <e3> ... <eN>)
    AK_TREE,
    // '<expr>
    AK_QUOTE,
    // `<expr>
    AK_BACKQUOTE,
    // ,<expr>
    AK_BQ_EVAL,
    // ,@<expr>
    AK_BQ_EXPAND,
};
typedef enum ASTKind ASTKind;

struct AST
{
    ASTKind kind;
    size_t children_count;
    Token *value;
    AST **children;
};

struct Parser
{
    char *file;
    Token *tokens;
};

/* Front-facing functions */
// Create a new lexer. 'file' can be NULL and is for
// error-reporting purposes only. Source must be non-null.
// Source will not be altered, it is read-only
Lexer *new_lexer(char *file, char *source);
// Create a new parser. 'tokens' refers to the head
// of a linked list of tokens
Parser *new_parser(char *file, Token *tokens);
// Lex all of the tokens from the source, storing them
// as a linked list with head stored at **head and tail
// at **tail
void lex_all(Lexer *lexer, Token **head, Token **tail);
// Parse the root of the program -- a sequence of S-expressions
AST *parse_root(Parser *parser);
// Parse one single S-expression
AST *parse(Parser *parser);

/* General helper functions */
// Allocate a new string that is a slice between begin and end
char *slice(char *begin, char *end);
// Is an allowed character in a name
int is_namechar(char ch);
// Is a base-8 digit
int is_odigit(char chr);
// Is a base-2 digit
int is_bdigit(char chr);
// Is prefix a prefix of str
int is_prefix(char *str, char *prefix);

/* Misc constructors */
Token *new_token(TokenKind kind, char *value);
AST *new_AST(ASTKind kind, Token *value);

/* Lexer functions */
// Assign the current locus of the lexer to the token
void assign_locus(Token *token, Lexer *lexer);
// Advance the lexer to the next character in source
void lexer_advance(Lexer *lexer);
// Obtain next token
Token *lexer_next(Lexer *lexer);
Token *lex_string(Lexer *lexer);
Token *lex_number(Lexer *lexer);
Token *lex_number_b16(Lexer *lexer);
Token *lex_number_b8(Lexer *lexer);
Token *lex_number_b2(Lexer *lexer);
Token *lex_number_b10(Lexer *lexer);
Token *lex_name(Lexer *lexer);

/* Parser functions */
// Expect token kind 'kind', return the consumed token, throw error otherwise
Token *expect(Parser *parser, TokenKind kind);
// Assert that there are tokens remaining to parse, throw error otherwise
Token *no_eof(Parser *parser);
AST *parse_value(Parser *parser);
// Quote: 'EXPR
AST *parse_quote(Parser *parser);
// Backquote: `EXPR
AST *parse_backquote(Parser *parser);
// Bq stands for 'backquote'
// bq-eval: ,EXPR
AST *parse_bq_eval(Parser *parser);
// bq-expand: ,@EXPR
AST *parse_bq_expand(Parser *parser);
AST *parse_tree(Parser *parser);

/* Error reporting & display */
void lexer_error(Lexer *lexer, char *efmt, ...);
void parser_error(Parser *parser, char *efmt, ...);
void print_AST(AST *tree);
const char *strtokkind(TokenKind kind);
const char *strASTkind(ASTKind kind);

/* Implementation */
#ifdef _SPLIB_IMPLEMENTATION

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

/* Parser implementation */

void print_AST_(AST *ast, size_t level)
{
    for (size_t i = 0; i < level; i++)
        putchar(' ');

    if (ast->value != NULL)
    {
        printf("%s %s\n", strASTkind(ast->kind), ast->value->value);
    }
    else
    {
        printf("%s\n", strASTkind(ast->kind));
    }

    if (ast->children_count)
    {
        for (size_t i = 0; i < ast->children_count; i++)
        {
            print_AST_(ast->children[i], level + 1);
        }
    }
}

const char *strASTkind(ASTKind kind)
{
    switch(kind)
    {
        case AK_VALUE: return "value";
        case AK_TREE: return "tree";
        case AK_QUOTE: return "quote";
        case AK_ROOT: return "root";
        case AK_BACKQUOTE: return "backquote";
        case AK_BQ_EVAL: return "bq-eval";
        case AK_BQ_EXPAND: return "bq-expand";
    }
    return "unknown AST kind";
}

void print_AST(AST* ast)
{
    print_AST_(ast, 0);
}

AST *new_AST(ASTKind kind, Token *value)
{
    AST *ast = malloc(sizeof(AST));
    ast->children_count = 0;
    ast->children = NULL;
    ast->value = value;
    ast->kind = kind;

    return ast;
}

Parser *new_parser(char *file, Token *tokens)
{
    Parser *parser = malloc(sizeof(Parser));
    parser->tokens = tokens;
    parser->file = file;
    return parser;
}

void parser_error(Parser *parser, char *efmt, ...)
{
    va_list valst;
    va_start(valst, efmt);

    if (parser->tokens == NULL)
    {
        fprintf(stderr, "[Parser error at %s EOF] ", parser->file);
        goto end;
    }

    Locus locus = parser->tokens->locus;

    fprintf(stderr, "[Parser error at %s %zu:%zu] ",
            locus.file,
            locus.line,
            locus.col);
end:
    vfprintf(stderr, efmt, valst); 

    va_end(valst);
}

Token *no_eof(Parser *parser)
{
    if (parser->tokens == NULL)
    {
        parser_error(parser, "Unexpected EOF\n");
        exit(1);
    }

    Token *token = parser->tokens;
    parser->tokens = parser->tokens->next;

    return token;
}

Token *expect(Parser *parser, TokenKind kind)
{
    if (parser->tokens == NULL)
    {
        parser_error(parser, "Unexpected EOF\n");
        exit(1);
    }

    if (parser->tokens->kind != kind)
    {
        parser_error(parser, "Wrong token kind, expected %s, got %s\n",
                strtokkind(kind),
                strtokkind(parser->tokens->kind));
        exit(1);
    }

    Token *token = parser->tokens;
    parser->tokens = parser->tokens->next;

    return token;
}

void print_AST(AST *tree);

AST *parse_value(Parser *parser)
{
    Token *token = no_eof(parser);

    if (token->kind == TK_NUMBER
            || token->kind == TK_STRING
            || token->kind == TK_NAME)
    {
        AST *ast = new_AST(AK_VALUE, token);
        return ast;
    }


    parser_error(parser, "Wrong token kind, expected name|string|number, got %s\n",
            strtokkind(token->kind));
    exit(1);
}

AST *parse_tree(Parser *parser)
{
    AST *tree = new_AST(AK_TREE, NULL);

    expect(parser, TK_LPAREN);

    AST *child = NULL;

    for (;;)
    {
        child = parse(parser);
        if (child == NULL) break;

        tree->children_count++;
        tree->children = reallocarray(tree->children, tree->children_count, sizeof(AST*));
        tree->children[tree->children_count-1] = child;

        if (parser->tokens->kind == TK_RPAREN) break;
    }

    expect(parser, TK_RPAREN);

    return tree;
}

AST *parse(Parser *parser)
{
    Token *token = parser->tokens;

    if (token == NULL) return NULL;

    if (token->kind == TK_NUMBER
            || token->kind == TK_STRING
            || token->kind == TK_NAME)
    {
        return parse_value(parser);
    }


    switch (token->kind)
    {
        case TK_TICK:
            return parse_quote(parser);
        case TK_BACKTICK:
            return parse_backquote(parser);
        case TK_COMMA:
            return parse_bq_eval(parser);
        case TK_COMMA_AT:
            return parse_bq_expand(parser);
        default:
            return parse_tree(parser);
    }

    assert(0 && "UNREACHABLE");
}

AST *parse_next_as_child_(Parser *parser)
{
    // AK_QUOTE is just a dummy here
    AST *nb = new_AST(AK_QUOTE, NULL);

    nb->children = malloc(sizeof(AST*));
    nb->children_count = 1;

    AST *child = parse(parser);

    nb->children[0] = child;

    return nb;
}

AST *parse_quote(Parser *parser)
{
    expect(parser, TK_TICK);

    AST *quote = parse_next_as_child_(parser);
    quote->kind = AK_QUOTE;

    return quote;
}

AST *parse_backquote(Parser *parser)
{
    expect(parser, TK_BACKTICK);

    AST *bquote = parse_next_as_child_(parser);
    bquote->kind = AK_BACKQUOTE;

    return bquote;
}

AST *parse_bq_eval(Parser *parser)
{
    expect(parser, TK_COMMA);

    AST *bqe = parse_next_as_child_(parser);
    bqe ->kind = AK_BQ_EVAL;

    return bqe;
}

AST *parse_bq_expand(Parser *parser)
{
    expect(parser, TK_COMMA_AT);

    AST *bqe = parse_next_as_child_(parser);
    bqe->kind = AK_BQ_EXPAND;

    return bqe;
}

AST *parse_root(Parser *parser)
{
    AST *root = new_AST(AK_ROOT, NULL);

    AST *child = NULL;

    while ((child = parse(parser)) != NULL)
    {
        root->children_count++;
        root->children = reallocarray(root->children, root->children_count, sizeof(AST*));
        root->children[root->children_count-1] = child;
    }

    return root;
}

#endif /* ifdef _SP_IMPLEMENTATION */

#endif /* ifndef _SP_H */
