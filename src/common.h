#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t sh_var;
typedef uint8_t  sh_byte;
typedef uint64_t sh_size;

#define SH_VAR_MAX  UINT64_MAX
#define SH_BYTE_MAX UINT8_MAX
#define SH_SIZE_MAX UINT64_MAX

#define KiB(n) ((n) * 1024ULL)
#define MiB(n) ((n) * 1024ULL * 1024ULL)
#define GiB(n) ((n) * 1024ULL * 1024ULL * 1024ULL)

#endif
