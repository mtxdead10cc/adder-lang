#ifndef SH_ARENA_H_
#define SH_ARENA_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct arena_t arena_t;
typedef struct arena_t {
    arena_t* next;
    ptrdiff_t size;
    ptrdiff_t capacity;
    uint8_t* data;
} arena_t;

arena_t* arena_create(ptrdiff_t size);
void     arena_destroy(arena_t* arena);
void     arena_dump(arena_t* arena);
void*    aalloc(arena_t* arena, ptrdiff_t size);
void*    arealloc(arena_t* arena, void* srcptr, ptrdiff_t size);

#define anew2(a, t)          (t*) aalloc(a, sizeof(t))
#define anew3(a, t, n)       (t*) aalloc(a, sizeof(t) * n)
#define anewx(a,b,c,d,...) d
#define anew(...)            anewx(__VA_ARGS__,anew3,anew2)(__VA_ARGS__)


#endif // SH_ARENA_H_