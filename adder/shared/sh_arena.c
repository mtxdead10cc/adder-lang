#include "sh_arena.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

#define ALIGNMENT sizeof(intptr_t) 
#define ARENA_MIN_SIZE (1024 * 16)

inline static ptrdiff_t get_padding(uint8_t* ptr) {
    return -(uintptr_t)ptr & (ALIGNMENT - 1);
}

arena_t* arena_create(ptrdiff_t size) {
    ptrdiff_t capacity = MAX(ARENA_MIN_SIZE, size);
    uint8_t* data = (uint8_t*) malloc( sizeof(uint8_t) * capacity );
    if( data == NULL ) {
        return NULL;
    }
    arena_t* a = (arena_t*) malloc( sizeof(arena_t) );
    if( a == NULL ) {
        free(data);
        return NULL;
    }
    a->data = data;
    a->size = 0;
    a->capacity = capacity;
    a->next = NULL;
    return a;
}

void arena_dump(arena_t* arena) {
    arena_t* current = arena;
    int indent = 1;
    while( current != NULL ) {
        printf("%*sarena\n", indent, " ");
        printf("%*s size:     %li\n", indent, " ", current->size);
        printf("%*s capacity: %li\n", indent, " ", current->capacity);
        printf("%*s next:     %p\n", indent, " ", (void*) current->next);
        current = current->next;
        indent ++;
    }
}

void arena_destroy(arena_t* arena) {
    arena_t* current = arena;
    while( current != NULL ) {
        free(current->data);
        arena_t* next = current->next;
        current->capacity = 0;
        current->size = 0;
        current->next = NULL;
        free(current);
        current = next;
    }
}

void* aalloc(arena_t* arena, ptrdiff_t size) {

    ptrdiff_t padding, available;
    arena_t* current = arena;

    while( current != NULL ) {
        padding = get_padding(current->data + current->size);
        available = current->capacity - current->size - padding;
        arena = current;
        if( available >= size )
            break;
        current = current->next;
    }

    if ( current == NULL ) {
        assert(arena->next == NULL);
        arena->next = arena_create(size);
        if( arena->next == NULL )
            return NULL;
        arena->next->size += size;
        return memset(arena->next->data, 0, size);
    }

    void *p = arena->data + arena->size + padding;
    arena->size += padding + size;
    return memset(p, 0, size);
}

void* arealloc(arena_t* arena, void* srcptr, ptrdiff_t size) {
    uint8_t* refptr = (uint8_t*) srcptr;
    
    ptrdiff_t srclen = -1;
    arena_t* current = arena;

    // TODO: improve by accepting scrptr == NULL

    // forward to top arena while
    // looking for source data len
    while( current != NULL ) {
        ptrdiff_t diff = refptr - current->data;
        if( diff < current->capacity && diff >= 0 ) {
            srclen = current->capacity - diff;
        }
        arena = current;
        current = current->next;
    }

    srclen = MIN(size, srclen);
    if( srclen < 0 )
        return NULL;

    void* destptr = aalloc(arena, size);
    if( destptr == NULL )
        return NULL;

    memcpy(destptr, srcptr, srclen);
    return destptr;
}

char* astrcopy(arena_t* arena, char* src, size_t len) {
    char* dest = (char*) aalloc(arena, ((len + 1) * sizeof(char)));
    if( dest == NULL )
        return NULL;
    memcpy(dest, src, len * sizeof(char));
    dest[len] = '\0';
    return dest;
}

char* asprint(arena_t* arena, const char* fmt, ...) {
    va_list length_args;
    va_start(length_args, fmt);
    va_list result_args;
    va_copy(result_args, length_args);

    int len = vsnprintf(NULL, 0, fmt, length_args);
    assert( len >= 0 );
    char* str = aalloc(arena, (len + 1) * sizeof(char));
    vsnprintf(str, len + 1, fmt, result_args);

    va_end(result_args);
    va_end(length_args);
    return str;
}

