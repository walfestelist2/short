#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "lexer.h"

static inline char peek (struct sh_lexer *L) {
    return L->src[L->curr];
}

static inline char consume (struct sh_lexer *L) {
    return L->src[L->curr++];
}

static inline char consume_n (struct sh_lexer *L, sh_size n) {
    char c = peek(L);

    L->curr += n;
    return c;
}

/* looks 1 token ahead */
static inline char look (struct sh_lexer *L) {
    return L->src[L->curr+1];
}

static inline const char *curr (struct sh_lexer *L) {
    return L->src + L->curr;
}

/* skips all the things that don't affect the program */
static inline void skip_void (struct sh_lexer *L) {
    while (peek(L) == ' ' || peek(L) == '\t') consume(L);

    if (peek(L) == '(') {
        consume(L); /* skipping the first paren */
        while (peek(L) != ')') {
            shL_assert(peek(L) != '(', "comment start inside a comment");
            shL_assert(peek(L) != '\0', "unterminated comment");
            consume(L);
        }
        consume(L); /* skipping the last paren */
    }

    while (peek(L) == ' ' || peek(L) == '\t') consume(L);
}

static void print (struct sh_lexer *L) {
    struct sh_tok *toks = (struct sh_tok*)L->toks.data;

    for (sh_size i = 0; toks[i].type != TK_EOF; i++) {
        printf("%s: %zu\n", sh_lex_to_string(toks[i].type), toks[i].value);
    }
}

static void init (struct sh_lexer *L, const char *src, sh_size capacity) {
    sh_arena_init(&L->toks, capacity);
    L->src = src;
    L->curr = 0;
}

static void push (struct sh_lexer *L, struct sh_tok tok) {
    struct sh_tok *tok_addr = sh_arena_alloc(&L->toks, sizeof(struct sh_tok));

    *tok_addr = tok;
}

static sh_var prefix_num (struct sh_lexer *L, const char *full_prefix) {
    char *endptr;

    errno = 0;
    sh_var num = strtoull(curr(L), &endptr, 10);

    shL_assert(endptr != curr(L), "expected %s number, got '%c'", full_prefix, peek(L));
    shL_assert(errno != ERANGE, "%s number overflow", full_prefix);

    sh_size num_len = endptr - curr(L);
    L->curr += num_len;

    return num;
}

static sh_var num (struct sh_lexer *L, int base) {
    char *endptr;

    errno = 0;
    sh_var num_ = strtoull(curr(L), &endptr, base);

    shL_assert(endptr != curr(L), "unterminated literal in num");
    shL_assert(errno != ERANGE, "literal overflow");

    sh_size num_len = endptr - curr(L);
    L->curr += num_len;

    shL_assert(peek(L) < '0' || peek(L) > '9', "expected a space before two literals");

    return num_;
}

static sh_var symbol (struct sh_lexer *L) {
    shL_assert(consume(L) == '\'', "unreachable because of how 'lit' function should work");

    char symb = consume(L);
    /* escape character handling */
    if (symb == '\\') {
        switch (peek(L)) {
            case 'n' : symb = '\n'  ; break;
            case 'e' : symb = '\033'; break;
            case 'b' : symb = '\b'  ; break;
            case 't' : symb = '\t'  ; break;
            case 'a' : symb = '\a'  ; break;
            case '\'': symb = '\''  ; break;
            case '\\': symb = '\\'  ; break;
            default: shL_error("unknown escape character: '%c'", peek(L));
        }
        consume(L); /* skipping the character */
    }
    shL_assert(peek(L) == '\'', "unterminated literal, expected \"'\", got '%c' (%d)", peek(L), peek(L));
    consume(L);
    return (sh_var)symb;
}

static sh_var lit (struct sh_lexer *L) {
    skip_void(L);

    char first = peek(L);

    if (first == '\0') {
        shL_error("expected literal");
    }

    if (first == '0' && look(L) == 'x') {
        /* hexadecimal */
        consume_n(L, 2);
        return num(L, 16);
    }

    if (first == '0' && look(L) == 'o') {
        /* octal */
        consume_n(L, 2);
        return num(L, 8);
    }

    if (first == '0' && look(L) == 'b') {
        /* binary */
        consume_n(L, 2);
        return num(L, 2);
    }

    if (first >= '0' && first <= '9') {
        /* decimal */
        return num(L, 10);
    }

    if (first == '\'') {
        /* ASCII literal */
        return symbol(L);
    }

    shL_error("expected literal, got '%c'", peek(L));
}

static struct sh_tok next (struct sh_lexer *L) {
    skip_void(L);

    switch (peek(L)) {
        case '\0': consume(L); return (struct sh_tok){.type = TK_EOF};
        case '\n': consume(L); return (struct sh_tok){.type = TK_NEWLINE};
        case '.' : consume(L); return (struct sh_tok){.type = TK_DOT};
        case '{' : consume(L); return (struct sh_tok){.type = TK_LBRACE};
        case '}' : consume(L); return (struct sh_tok){.type = TK_RBRACE};
        case '[' : consume(L); return (struct sh_tok){.type = TK_LBRACKET};
        case ']' : consume(L); return (struct sh_tok){.type = TK_RBRACKET};
        case ':' : consume(L); return (struct sh_tok){.type = TK_COLON};
        case '=' : consume(L); return (struct sh_tok){.type = TK_ASSIGN};
        case '+' : consume(L); return (struct sh_tok){.type = TK_PLUS};
        case '-' : consume(L); return (struct sh_tok){.type = TK_MINUS};
        case '*' : consume(L); return (struct sh_tok){.type = TK_STAR};
        case '/' : consume(L); return (struct sh_tok){.type = TK_SLASH};
        case '%' : consume(L); return (struct sh_tok){.type = TK_PERCENT};
        case '^' : consume(L); return (struct sh_tok){.type = TK_CARET};
        case '|' : consume(L); return (struct sh_tok){.type = TK_PIPE};
        case '?' : consume(L); return (struct sh_tok){.type = TK_QUESTION};

        case 'v' : consume(L); return (struct sh_tok){.type = TK_VAR, .value = prefix_num(L, "variable")};
        case 'b' : consume(L); return (struct sh_tok){.type = TK_BYTE, .value = prefix_num(L, "byte")};
        case 'l' : consume(L); return (struct sh_tok){.type = TK_LABEL, .value = prefix_num(L, "label")};
        case 'i' : consume(L); return (struct sh_tok){.type = TK_IF};
        case 'j' : consume(L); return (struct sh_tok){.type = TK_JUMP};
        case 'w' : consume(L); return (struct sh_tok){.type = TK_WRITE};
        case 'r' : consume(L); return (struct sh_tok){.type = TK_READ};
        case 'e' : consume(L); return (struct sh_tok){.type = TK_EXIT, .value = lit(L)};

        case '<' : {
            consume(L);
            if (peek(L) == '<') {
                consume(L);
                return (struct sh_tok){.type = TK_LSHIFT};
            }
            if (peek(L) == '=') {
                consume(L);
                return (struct sh_tok){.type = TK_LE};
            }

            return (struct sh_tok){.type = TK_LANGLE};
        };
        case '>' : {
            consume(L);
            if (peek(L) == '>') {
                consume(L);
                return (struct sh_tok){.type = TK_RSHIFT};
            }
            if (peek(L) == '=') {
                consume(L);
                return (struct sh_tok){.type = TK_GE};
            }
            return (struct sh_tok){.type = TK_RANGLE};
        };
        default: {
            if ((peek(L) >= '0' && peek(L) <= '9') || peek(L) == '\'') 
                return (struct sh_tok){.type = TK_LIT, lit(L)};

            shL_error("unexpected symbol: '%c'", peek(L));
        }
    }
}

struct sh_lexer sh_lex (const char *src) {
    struct sh_lexer L;
    init(&L, src, MiB(1));

    while (1) {
        struct sh_tok next_tok = next(&L);

        if (next_tok.type == TK_EOF) break;

        push(&L, next_tok);
    }

    // print(&L);

    return L;
}
