#include <stdarg.h>
#include <stdlib.h>

#include "colors.h"
#include "gen.h"
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

void sh_error (const char *msg, ...) {
    va_list args;
    va_start(args, msg);

    errorf(COLOR_RED "[ERROR]" COLOR_RESET " ");
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
