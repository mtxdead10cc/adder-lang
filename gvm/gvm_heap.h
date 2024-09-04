#ifndef GVM_HEAP_H_
#define GVM_HEAP_H_

#include <stdbool.h>
#include <stdint.h>
#include "gvm_types.h"

#define HEAP_TO_PAGE_INDEX(HI) ((HI) / (sizeof(uint64_t) * CHAR_BIT))
#define HEAP_TO_BIT_INDEX(HI) ((HI) % (sizeof(uint64_t) * CHAR_BIT))
#define MK_CHUNK_MASK(N) (~(0xFFFFFFFFFFFFFFFFUL << N))
#define CALC_GC_MARK_U64_COUNT(VAL_COUNT) (1 + ((VAL_COUNT) / (sizeof(uint64_t) * CHAR_BIT)))

void heap_gc_collect(gvm_t* vm);
void heap_print_usage(gvm_t* vm);
val_t heap_alloc_array(gvm_t* vm, int val_count);


#endif // GVM_HEAP_H_
