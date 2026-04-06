# SPLIB

Single-header drop-in lexer & parser for S-expressions in C.

## Usage

Define `_SPLIB_IMPLEMENTATION` before including `splib.h` in exactly one
translation unit. Other files should include `splib.h` without the macro.

```c
#define _SPLIB_IMPLEMENTATION
#include "splib.h"
```

### Quick example

```c
char *source = "(define (square x) (* x x))";

Lexer *lexer = new_lexer("example", source);

Token *head = NULL, *tail = NULL;
lex_all(lexer, &head, &tail);

Parser *parser = new_parser("example", source, head);
AST *root = parse_root(parser);

print_AST(root);

destroy_AST(root);
destroy_all_tokens(head);
destroy_parser(parser);
destroy_lexer(lexer);
```

## Supported syntax

### Atoms

| Syntax | Token | Examples |
|---|---|---|
| Names | `TK_NAME` | `foo`, `+`, `a.b`, `set!` |
| Strings | `TK_STRING` | `"hello, world"` |
| Numbers (base 10) | `TK_NUMBER` | `42`, `3.14`, `+10`, `-5.0` |
| Numbers (base 16) | `TK_NUMBER` | `0xffae0BD` |
| Numbers (base 8) | `TK_NUMBER` | `0o07050071` |
| Numbers (base 2) | `TK_NUMBER` | `0b010101` |
| Character strings | `TK_CHAR_STRING` | `#\a`, `#\space`, `#\(` |

### Compound expressions

| Syntax | AST kind | Example |
|---|---|---|
| Lists | `AK_TREE` | `(a b c)` |
| Dotted pairs | `AK_DOTTED` | `(a . b)`, `(a b . c)` |
| Quote | `AK_QUOTE` | `'expr` |
| Backquote | `AK_BACKQUOTE` | `` `expr `` |
| Unquote | `AK_BQ_EVAL` | `,expr` |
| Splice-unquote | `AK_BQ_EXPAND` | `,@expr` |

Dotted pairs are automatically collapsed: `(a . (b . c))` becomes `(a b . c)`.
If the cdr is a proper list, the result is a proper list: `(a . (b c))` becomes `(a b c)`.

## Interface

### Constructors and destructors

- `new_lexer(file, source)` -- create a lexer; `file` may be NULL (used for error messages only)
- `new_parser(file, source, tokens)` -- create a parser from a token linked list
- `destroy_lexer(lexer)` -- free the lexer (does not free source)
- `destroy_parser(parser)` -- free the parser (does not free tokens)
- `destroy_token(token)` -- free a single token and its value
- `destroy_all_tokens(head)` -- free an entire token linked list
- `destroy_AST(ast)` -- free an AST node and all children (does not free tokens)

All destroy functions accept NULL and always return NULL.

### Lexing

- `lex_all(lexer, &head, &tail)` -- lex all tokens into a linked list
- Check `lexer->balance` after lexing to detect unmatched parentheses

### Parsing

- `parse_root(parser)` -- parse a sequence of S-expressions (returns `AK_ROOT`)
- `parse(parser)` -- parse a single S-expression
- `print_AST(ast)` -- print the AST tree to stdout

## License

Public domain (Unlicense).
