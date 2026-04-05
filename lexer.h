#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>

typedef struct Locus Locus;
typedef enum TokenKind TokenKind;
typedef struct Token Token;
typedef struct Lexer Lexer;

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

#endif
