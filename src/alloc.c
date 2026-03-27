#include <stdlib.h>

#include "alloc.h"
#include "utils.h"

void *sh_malloc (sh_size size) {
    void *ptr = malloc((size_t)size);
    if (ptr == NULL) fatalf("Error: out of memory\n");

    return ptr;
}

void *sh_realloc (void *ptr, sh_size size) {
    void *new_ptr = realloc(ptr, (size_t)size);
    if (new_ptr == NULL) fatalf("Error: out of memory\n");

    return new_ptr;
}

void sh_free (void *ptr) {
    free(ptr);
}
