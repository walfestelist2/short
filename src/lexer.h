#ifndef LEXER_H
#define LEXER_H

#include "arena.h"
#include "utils.h"

enum sh_tok_type {
    /* Endlines */
    TK_EOF,
    TK_NEWLINE,
    TK_DOT,

    TK_LIT,         /* sh_var literal */

    TK_VAR,         /* 'v' */
    TK_BYTE,        /* 'b' */
    TK_LABEL,       /* 'l' */
    TK_LBRACE,      /* '{' */
    TK_RBRACE,      /* '}' */
    TK_LBRACKET,    /* '[' */
    TK_RBRACKET,    /* ']' */
    TK_LANGLE,      /* '<' */
    TK_RANGLE,      /* '>' */

    TK_COLON,       /* ':' */

    TK_ASSIGN,      /* '=' */
    TK_PLUS,        /* '+' */
    TK_MINUS,       /* '-' */
    TK_STAR,        /* '*' */
    TK_SLASH,       /* '/' */
    TK_PERCENT,     /* '%' */
    TK_CARET,       /* '^' */
    TK_PIPE,        /* '|' */
    TK_AND,         /* '&' */
    TK_LSHIFT,      /* "<<" */
    TK_RSHIFT,      /* ">>" */
    TK_QUESTION,    /* '?' */

    TK_IF,          /* 'i' */
    TK_JUMP,        /* 'j' */
    TK_EQUAL,       /* "==" */
    TK_NOT_EQUAL,   /* "!=" */
    TK_LE,          /* "<=" */
    TK_GE,          /* ">=" */

    TK_WRITE,       /* 'w' */
    TK_READ,        /* 'r' */
    TK_EXIT         /* 'e' */
};

struct sh_tok {
    enum sh_tok_type type;
    sh_var value; /* optional */
};

struct sh_lexer {
    struct sh_arena toks;

    const char *src;
    sh_size curr;
};

const char *sh_lex_to_string (enum sh_tok_type type);
struct sh_lexer sh_lex(const char *src);

#endif /* LEXER_H */
