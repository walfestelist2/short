#include <stdint.h>
#include <stdlib.h>

#include "alloc.h"
#include "arena.h"
#include "utils.h"

void sh_arena_init (struct sh_arena *a, sh_size size) {
    a->data = sh_malloc(size);
    a->capacity = size;
    a->count = 0;
}

void *sh_arena_alloc (struct sh_arena *a, sh_size size) {
    sh_size ptr_size = sizeof(void*);
    sh_size aligned_count = (a->count + ptr_size - 1) & ~(ptr_size - 1);

    sh_assert(aligned_count + size < a->capacity, "out of arena memory");

    void *ptr = (uint8_t*)a->data + aligned_count;
    a->count = aligned_count + size;
    return ptr;
}

void sh_arena_remove (struct sh_arena *a) {
    free(a->data);
    a->data = NULL;
    a->capacity = 0;
    a->count = 0;
}
