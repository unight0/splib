#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _SPLIB_IMPLEMENTATION
#include "splib.h"

// TO DO:
// Comments
// Option to disable error reporting/crashes (if possible)

void parse_one(char *source)
{
    Lexer *lexer = new_lexer(NULL, source);

    Token *tokens = NULL, *tokens_tail = NULL;

    lex_all(lexer, &tokens, &tokens_tail);

    if (lexer->balance)
    {
        lexer_error(lexer, "Unbalanced parenthesis\n");
        exit(1);
    }

    Parser *parser = new_parser(NULL, source, tokens);

    AST *root = parse_root(parser);
    
    print_AST(root);

    root = destroy_AST(root);

    destroy_all_tokens(tokens);

    tokens = tokens_tail = NULL;

    parser = destroy_parser(parser);
    lexer = destroy_lexer(lexer);
}

char *read_stdin(void)
{
    size_t input_size = 1;
    char *input = malloc(1);
    *input = 0;
    char buffer[2048];

    while(!feof(stdin) && !ferror(stdin))
    {
        ssize_t got = fread(buffer, 1, 2048, stdin);
        if (got < 0) break;
        input = realloc(input, input_size + got);
        memcpy(input + input_size - 1, buffer, got);
        input_size += got;
        input[input_size - 1] = 0;
    }

    return input;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        char *input = read_stdin();
        parse_one(input);
        free(input);
        return 0;
    }

    for (int i = 1; i < argc; i++)
    {
        parse_one(argv[i]);
    }

    //char *source = "(a (+ b c) (* e f) 'x \"hello, world\" 10 20 30 0xffae0BD 0o07050071 0b010101110) (a `(b ,e ,@l))";
}
