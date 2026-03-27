#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "lexer.h"
#include "utils.h"

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
            sh_assert(peek(L) != '(', "comment start inside a comment");
            sh_assert(peek(L) != '\0', "unterminated comment");
            consume(L);
        }
        consume(L); /* skipping the last paren */
    }

    while (peek(L) == ' ' || peek(L) == '\t') consume(L);
}

const char *sh_lex_to_string (enum sh_tok_type type) {
    switch (type) {
        case TK_EOF:        return "EOF";
        case TK_NEWLINE:    return "NEWLINE";
        case TK_DOT:        return "DOT";
        case TK_LIT:        return "LIT";
        case TK_VAR:        return "VAR";
        case TK_BYTE:       return "BYTE";
        case TK_LABEL:      return "LABEL";
        case TK_LBRACE:     return "LBRACE";
        case TK_RBRACE:     return "RBRACE";
        case TK_LBRACKET:   return "LBRACKET";
        case TK_RBRACKET:   return "RBRACKET";
        case TK_LANGLE:     return "LANGLE";
        case TK_RANGLE:     return "RANGLE";
        case TK_ASSIGN:     return "ASSIGN";
        case TK_COLON:      return "COLON";
        case TK_PLUS:       return "PLUS";
        case TK_MINUS:      return "MINUS";
        case TK_STAR:       return "STAR";
        case TK_SLASH:      return "SLASH";
        case TK_PERCENT:    return "PERCENT";
        case TK_CARET:      return "CARET";
        case TK_PIPE:       return "PIPE";
        case TK_LSHIFT:     return "LSHIFT";
        case TK_RSHIFT:     return "RSHIFT";
        case TK_EQUAL:      return "EQUAL";
        case TK_NOT_EQUAL:  return "NOT_EQUAL";
        case TK_LE:         return "LE";
        case TK_GE:         return "GE";
        case TK_WRITE:      return "WRITE";
        case TK_READ:       return "READ";
        case TK_EXIT:       return "EXIT";
        default:            return "UNIMPLEMENTED";
    }
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

    sh_assert(endptr != curr(L), "expected %s number, got '%c'", full_prefix, peek(L));
    sh_assert(errno != ERANGE, "%s number overflow", full_prefix);

    sh_size num_len = endptr - curr(L);
    L->curr += num_len;

    return num;
}

static sh_var num (struct sh_lexer *L, int base) {
    char *endptr;

    errno = 0;
    sh_var num_ = strtoull(curr(L), &endptr, base);

    sh_assert(endptr != curr(L), "unterminated literal");
    sh_assert(errno != ERANGE, "literal overflow");

    sh_size num_len = endptr - curr(L);
    L->curr += num_len;

    sh_assert(peek(L) < '0' || peek(L) > '9', "expected a space before two literals");

    return num_;
}

static sh_var symbol (struct sh_lexer *L) {
    sh_assert(consume(L) == '\'', "unreachable because of how 'lit' function should work");

    char symb = consume(L);
    sh_assert(peek(L) == '\'', "unterminated literal");
    consume(L);
    return symb;
}

static sh_var lit (struct sh_lexer *L) {
    skip_void(L);

    char first = peek(L);

    if (first == '\0') {
        sh_error("expected literal");
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

    sh_error("expected literal, got '%c'", peek(L));
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

        case 'v' : consume(L); return (struct sh_tok){.type = TK_VAR, .value = prefix_num(L, "variable")};
        case 'b' : consume(L); return (struct sh_tok){.type = TK_BYTE, .value = prefix_num(L, "byte")};
        case 'l' : consume(L); return (struct sh_tok){.type = TK_LABEL, .value = prefix_num(L, "label")};
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

            sh_error("unexpected symbol: '%c'", peek(L));
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

    print(&L);

    return L;
}
