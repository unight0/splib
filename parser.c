#include "sp.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

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
