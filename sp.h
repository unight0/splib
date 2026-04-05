#ifndef SP_H
#define SP_H

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
    AK_ROOT,
    AK_VALUE,
    AK_TREE,
    AK_QUOTE,
    AK_BACKQUOTE,
    AK_BQ_EVAL,
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

#endif
