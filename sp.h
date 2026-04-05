#ifndef SP_H
#define SP_H

#include <stdlib.h>

typedef struct Locus Locus;
typedef enum TokenKind TokenKind;
typedef struct Token Token;
typedef struct Lexer Lexer;
typedef struct AST AST;
typedef enum ASTKind ASTKind;
typedef struct Parser Parser;

struct Locus
{
    char *file;
    size_t line, col;
};

enum TokenKind
{
    TK_LPAREN,
    TK_RPAREN,

    TK_STRING,
    TK_NUMBER,
    TK_NAME,

    TK_TICK,
    TK_BACKTICK,
    TK_COMMA,
    TK_COMMA_AT,
};

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

// Is an allowed character in a name
int is_namechar(char ch);
// Is prefix a prefix of str
int is_prefix(char *str, char *prefix);
// Is a base-8 digit
int is_odigit(char chr);
// Is a base-2 digit
int is_bdigit(char chr);
// Convert TokenKind to string
const char *strtokkind(TokenKind kind);
// Allocate a new string that is a slice
// between begin and end
char *slice(char *begin, char *end);
Token *new_token(TokenKind kind, char *value);
void assign_locus(Token *token, Lexer *lexer);
Lexer *new_lexer(char *file, char *source);
void lexer_advance(Lexer *lexer);
void lexer_error(Lexer *lexer, char *efmt, ...);
Token *lexer_next(Lexer *lexer);
Token *lex_string(Lexer *lexer);
Token *lex_number(Lexer *lexer);
Token *lex_number_b16(Lexer *lexer);
Token *lex_number_b8(Lexer *lexer);
Token *lex_number_b2(Lexer *lexer);
Token *lex_number_b10(Lexer *lexer);
Token *lex_name(Lexer *lexer);
void lex_all(Lexer *lexer, Token **head, Token **tail);

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
