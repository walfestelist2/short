#ifndef ALLOC_H
#define ALLOC_H

#include "utils.h"

void *sh_malloc (sh_size size);
void *sh_realloc (void *ptr, sh_size size);
void sh_free (void *ptr);

#endif /* ALLOC_H */
