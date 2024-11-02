#ifndef SH_ARENA_H_
#define SH_ARENA_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include "sh_types.h"


arena_t* arena_create(ptrdiff_t size);
void     arena_destroy(arena_t* arena);
void     arena_dump(arena_t* arena);
void*    aalloc(arena_t* arena, ptrdiff_t size);
void*    arealloc(arena_t* arena, void* srcptr, ptrdiff_t size);
char*    astrcopy(arena_t* arena, char* src, size_t len);
char*    asprintf(arena_t* arena, const char* fmt, ...);

#endif // SH_ARENA_H_