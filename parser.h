#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct AST AST;
typedef enum ASTKind ASTKind;
typedef struct Parser Parser;

enum ASTKind
{
    AK_ROOT,
    AK_VALUE,
    AK_TREE,
    AK_QUOTE,
    AK_BACKQUOTE,
    AK_BQ_EVAL,
    AK_BQ_EXPAND,
};

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

AST *new_AST(ASTKind kind, Token *value);
Parser *new_parser(char *file, Token *tokens);
Token *expect(Parser *parser, TokenKind kind);
const char *strASTkind(ASTKind kind);
void print_AST(AST *tree);
void parser_error(Parser *parser, char *efmt, ...);
Token *no_eof(Parser *parser);
AST *parse_value(Parser *parser);
AST *parse_quote(Parser *parser);
AST *parse_backquote(Parser *parser);
AST *parse_bq_eval(Parser *parser);
AST *parse_bq_expand(Parser *parser);
AST *parse_tree(Parser *parser);
AST *parse(Parser *parser);
AST *parse_root(Parser *parser);

#endif
