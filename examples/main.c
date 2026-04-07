#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _SPLIB_IMPLEMENTATION
#include "../splib.h"

SSerializer *new_sserializer()
{
    SSerializer *ser = malloc(sizeof(SSerializer));
    ser->size = 0;
    ser->capacity = 16;
    ser->output = malloc(16);
    *ser->output = 0;

    return ser;
}

SSerializer *destroy_sserializer(SSerializer *ser)
{
    if (ser == NULL) return NULL;

    free(ser->output);
    free(ser);

    return NULL;
}

void sserializer_append(SSerializer *ser, char *str)
{
    if (str == NULL) return;

    size_t str_sz = strlen(str);
    size_t need_capacity = ser->size + str_sz;

    if (need_capacity > ser->capacity)
    {
        size_t doubled = ser->capacity * 2;
        ser->capacity = doubled >= need_capacity ? doubled : need_capacity;
        ser->output = realloc(ser->output, ser->capacity);
    }

    strncat(ser->output, str, ser->capacity);
    ser->size += str_sz;
}

// These return the string that was appended to output
char *serialize_AST(SSerializer *ser, AST *ast)
{
    assert(ser != NULL);
    assert(ast != NULL);

    switch(ast->kind)
    {
        case AK_VALUE: return serialize_value(ser, ast);
        case AK_TREE: return serialize_tree(ser, ast);
        case AK_QUOTE: return serialize_quote(ser, ast);
        case AK_ROOT: return serialize_root(ser, ast);
        case AK_BACKQUOTE: return serialize_backquote(ser, ast);
        case AK_BQ_EVAL: return serialize_bq_eval(ser, ast);
        case AK_BQ_EXPAND: return serialize_bq_expand(ser, ast);
        case AK_DOTTED: return serialize_dotted(ser, ast);
    }

    assert(0 && "UNREACHABLE");
}

#define _SP_SERIALIZER(body)\
    {\
    char *_position = ser->output + ser->size;\
    body;\
    return _position;\
    }

char *serialize_root(SSerializer *ser, AST *root)
    _SP_SERIALIZER
({
    for (size_t i = 0; i < root->children_count; i++)
    {
        serialize_AST(ser, root->children[i]);
        if (i < root->children_count - 1)
            sserializer_append(ser, " ");
    }
})

char *serialize_value(SSerializer *ser, AST *value)
    _SP_SERIALIZER
({
    // value->value->value is hilarious...
    sserializer_append(ser, value->value->value);
})

char *serialize_tree(SSerializer *ser, AST *ast)
    _SP_SERIALIZER
({
    sserializer_append(ser, "(");

    for (size_t i = 0; i < ast->children_count; i++)
    {
        serialize_AST(ser, ast->children[i]);
        if (i < ast->children_count - 1)
            sserializer_append(ser, " ");
    }

    sserializer_append(ser, ")");
})

char *serialize_dotted(SSerializer *ser, AST *ast)
    _SP_SERIALIZER
({
    sserializer_append(ser, "(");

    assert(ast->children_count >= 2);

    size_t i;
    for (i = 0; i < ast->children_count - 1; i++)
    {
        serialize_AST(ser, ast->children[i]);
        sserializer_append(ser, " ");
    }

    sserializer_append(ser, ". ");
    
    serialize_AST(ser, ast->children[i]);

    sserializer_append(ser, ")");
})

#define _SP_QUOTELIKE_SERIALIZER(str)\
    _SP_SERIALIZER\
    ({\
        assert(ast->children_count);\
        sserializer_append(ser, (str));\
        serialize_AST(ser, ast->children[0]);\
    })

char *serialize_quote(SSerializer *ser, AST *ast)
    _SP_QUOTELIKE_SERIALIZER("'");

char *serialize_backquote(SSerializer *ser, AST *ast)
    _SP_QUOTELIKE_SERIALIZER("`");

char *serialize_bq_eval(SSerializer *ser, AST *ast)
    _SP_QUOTELIKE_SERIALIZER(",");

char *serialize_bq_expand(SSerializer *ser, AST *ast)
    _SP_QUOTELIKE_SERIALIZER(",@");

void parse_one(char *source)
{
    Lexer *lexer = new_lexer(NULL, source);

    Token *tokens = NULL, *tokens_tail = NULL;

    if (lex_all(lexer, &tokens, &tokens_tail))
    {
        exit(1);
    }

    if (lexer->balance)
    {
        lexer_error(lexer, "Unbalanced parenthesis\n");
        exit(1);
    }

    Parser *parser = new_parser(NULL, source, tokens);

    AST *root = parse_root(parser);

    if (parser->error)
    {
        destroy_AST(root);
        destroy_all_tokens(tokens);
        destroy_parser(parser);
        destroy_lexer(lexer);
        exit(1);
    }

    print_AST(root);

    SSerializer *ser = new_sserializer();

    serialize_AST(ser, root);

    printf("%s\n", ser->output);

    ser = destroy_sserializer(ser);

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
