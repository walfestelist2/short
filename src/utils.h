#ifndef UTILS_H
#define UTILS_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "lexer.h"
#include "arena.h"

struct sh_gen;

#define errorf(msg, ...) fprintf(stderr, msg __VA_OPT__(,) __VA_ARGS__)
#define fatalf(msg, ...) do { errorf(msg __VA_OPT__(,) __VA_ARGS__); sh_exit(1); } while (0)

#define verrorf(msg, args) vfprintf(stderr, msg, args)
#define vfatalf(msg, args) do { verrorf(msg, args); sh_exit(1); } while (0)

void sh_exit(int code);

void sh_assert(int cond, const char *msg, ...);
void shL_assert(int cond, const char *msg, ...);
void shG_assert(struct sh_gen *G, int cond, const char *msg, ...);

void sh_error(const char *msg, ...);
void shL_error(const char *msg, ...);
void shG_error(struct sh_gen *G, const char *msg, ...);

#endif /* UTILS_H */
