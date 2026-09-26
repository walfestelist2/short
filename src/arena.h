#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

#include "common.h"

/* storage for different types of data (aligned by sizeof void* ) */
struct sh_arena {
    uint8_t *data;      /* byte pointer to the data */
    sh_size count;      /* first free byte from the start */
    sh_size capacity;   /* how many bytes it can store */
};

void sh_arena_init (struct sh_arena *a, sh_size size);
void *sh_arena_alloc (struct sh_arena *a, sh_size size);
void sh_arena_remove (struct sh_arena *a);

#endif /* ARENA_H */
