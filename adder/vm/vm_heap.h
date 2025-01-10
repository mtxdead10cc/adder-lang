#ifndef VM_HEAP_H_
#define VM_HEAP_H_

#include <stdbool.h>
#include <stdint.h>
#include "vm_types.h"
#include "sh_types.h"

#define HEAP_TO_PAGE_INDEX(HI) ((HI) / (sizeof(uint64_t) * CHAR_BIT))
#define HEAP_TO_BIT_INDEX(HI) ((HI) % (sizeof(uint64_t) * CHAR_BIT))
#define MK_CHUNK_MASK(N) (~(0xFFFFFFFFFFFFFFFFUL << N))
#define CALC_GC_MARK_U64_COUNT(VAL_COUNT) (1 + ((VAL_COUNT) / (sizeof(uint64_t) * CHAR_BIT)))

void heap_gc_collect(vm_t* vm);
void heap_print_usage(vm_t* vm);
array_t heap_array_alloc(vm_t* vm, int val_count);
int heap_array_copy_to(vm_t* vm, val_t* src, int length, array_t dest);
void heap_clear(vm_t* vm);

#endif // VM_HEAP_H_
