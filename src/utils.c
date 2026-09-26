#include <stdarg.h>
#include <stdlib.h>

#include "colors.h"
#include "gen.h"
#include "lexer.h"
#include "utils.h"

void sh_exit (int code) {
    exit(code);
}

void sh_assert (int cond, const char *format, ...) {
    va_list args;
    va_start(args, format);

    if (!cond) {
        errorf(COLOR_RED "[ERROR]" COLOR_RESET " ");
        verrorf(format, args);
        fatalf("\n");
    }

    va_end(args);
}

void shL_assert (int cond, const char *format, ...) {
    va_list args;
    va_start(args, format);

    if (!cond) {
        errorf(COLOR_RED "[LEXER ERROR]" COLOR_RESET " ");
        verrorf(format, args);
        fatalf("\n");
    }

    va_end(args);
}

void shG_assert (struct sh_gen *G, int cond, const char *format, ...) {
    va_list args;
    va_start(args, format);

    if (!cond) {
        errorf(COLOR_RED "[GENERATOR ERROR]" COLOR_RESET " ");
		errorf("line %zu: ", G->line);
        verrorf(format, args);
        fatalf("\n");
    }

    va_end(args);
}
void sh_error (const char *msg, ...) {
    va_list args;
    va_start(args, msg);

    errorf(COLOR_RED "[error]" COLOR_RESET " ");
    verrorf(msg, args);
    fatalf("\n");

    va_end(args);
}

void shL_error (const char *msg, ...) {
    va_list args;
    va_start(args, msg);

    errorf(COLOR_RED "[LEXER ERROR]" COLOR_RESET " ");
    verrorf(msg, args);
    fatalf("\n");

    va_end(args);
}

void shG_error (struct sh_gen *G, const char *msg, ...) {
    va_list args;
    va_start(args, msg);

    errorf(COLOR_RED "[GENERATOR ERROR]" COLOR_RESET " ");
    errorf("line %zu: ", G->line);
    verrorf(msg, args);
    fatalf("\n");
    va_end(args);
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
        case TK_QUESTION:   return "QUESTION";
        case TK_IF:         return "IF";
        case TK_JUMP:       return "JUMP";
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
