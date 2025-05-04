#include "vm_heap.h"
#include "vm.h"
#include "sh_types.h"
#include "sh_value.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sh_log.h>

#define _MAX(A,B) ((A) > (B) ? (A) : (B))
#define _MIN(A,B) ((A) < (B) ? (A) : (B))

inline static void put_mark(uint64_t* marks, int heap_index) {
    uint64_t mask_index = HEAP_TO_PAGE_INDEX(heap_index);
    uint64_t mask_bit = HEAP_TO_BIT_INDEX(heap_index);
    marks[mask_index] |= (1UL << mask_bit);
}

void heap_clear(vm_t* vm) {
    uint64_t* gc_marks = vm->mem.heap.gc_marks;
    int heapsize = vm->mem.heap.size;
    memset(gc_marks, 0, CALC_GC_MARK_U64_COUNT(heapsize) * sizeof(uint64_t));
}

void heap_gc_mark_used(vm_t* vm, val_t* checkmem, int val_count) {
    val_addr_t virt_addr_heap = MEM_MK_PROGR_ADDR(vm->mem.stack.size);
    // mark all references
    for(int i = 0; i < val_count; i++) {
        val_t value = checkmem[i];
        if( value.type != VAL_ARRAY ) {
            continue;
        }
        array_t array = val_into_array(value);
        if( array.address < virt_addr_heap ) {
            continue;
        }
        if( array.length <= 0 ) {
            continue;
        }
        int heap_start = (int) array.address - (int) virt_addr_heap;
        for(int j = 0; j < array.length; j++) {
            put_mark(vm->mem.heap.gc_marks, heap_start + j);
        }
        // call recursively (arrays inside array)
        heap_gc_mark_used(vm,
            vm->mem.heap.values + heap_start + 1,
            array.length - 1);
    }
}

void heap_gc_collect(vm_t* vm) {
    // clear all usage bits 
    memset(vm->mem.heap.gc_marks, 0, CALC_GC_MARK_U64_COUNT(vm->mem.heap.size) * sizeof(uint64_t));
    // mark all references from the stack
    heap_gc_mark_used(vm, vm->mem.stack.values, vm->mem.stack.top + 1);
}

void heap_print_usage(vm_t* vm) {
    int pages = CALC_GC_MARK_U64_COUNT(vm->mem.heap.size);
    int num_bits = sizeof(uint64_t) * CHAR_BIT;
    define_cstr(str, 2048);
    for(int i = 0; i < pages; i++) {
        cstr_append_fmt(str, "  ");
        for(int j = 0; j < num_bits; j++) {
            uint64_t marks = vm->mem.heap.gc_marks[i];
            bool on = ((1UL << j) & marks) > 0;
            cstr_append_fmt(str, "%s", on ? "1" : "0");
        }
        cstr_append_fmt(str, "\n");
    }
    sh_log_info("HEAP USAGE BITS\n%s", str.ptr);
}

int heap_get_used(vm_t* vm) {
    int used = 0;
    int pages = CALC_GC_MARK_U64_COUNT(vm->mem.heap.size);
    int num_bits = sizeof(uint64_t) * CHAR_BIT;
    for(int i = 0; i < pages; i++) {
        for(int j = 0; j < num_bits; j++) {
            uint64_t marks = vm->mem.heap.gc_marks[i];
            if(((1UL << j) & marks) > 0) {
                used += 8;
            }
        }
    }
    return used;
}

int heap_find_small_chunk(vm_t* vm, int value_count) {
    int num_bits_per_page = sizeof(uint64_t) * CHAR_BIT;
    int num_pages = CALC_GC_MARK_U64_COUNT(vm->mem.heap.size);
    int num_bits = num_bits_per_page - value_count;
    uint64_t chunk_mask = MK_CHUNK_MASK(value_count);
    for(int page_index = 0; page_index < num_pages; page_index++) {
        uint64_t page = vm->mem.heap.gc_marks[page_index];
        for(int bit = 0; bit < num_bits; bit++) {
            if( (page & (chunk_mask << bit)) == 0 ){
                return (num_bits_per_page * page_index) + bit;
            }
        }
    }
    return -1;
}

int heap_count_empty_pages(vm_t* vm, int page_index, int num_pages) {
    if( page_index >= num_pages ) {
        return -1;
    }
    int count = 0;
    while (page_index < num_pages) {
        if( vm->mem.heap.gc_marks[page_index++] == 0 ) {
            count ++;
        } else {
            return count;
        }
    }
    return count;
}

int heap_find_large_chunk(vm_t* vm, int value_count) {
    
    if( value_count > vm->mem.heap.size ) {
        return -1;
    }

    int num_bits_per_page = sizeof(uint64_t) * CHAR_BIT;
    int num_pages_total = CALC_GC_MARK_U64_COUNT(vm->mem.heap.size);
    int num_req_pages = CALC_GC_MARK_U64_COUNT(value_count) - 1;
    int num_trailing = value_count - (num_req_pages * num_bits_per_page);
    int max_check_page_count = num_pages_total - num_req_pages;

    int page_index = 0;
    while( page_index < max_check_page_count ) {
        int num_free = heap_count_empty_pages(vm, page_index, num_pages_total);
        if( num_free > num_req_pages ) {
            // just return the page_index
            return page_index * num_bits_per_page;
        } else if( num_free == num_req_pages ) {
            // check if trailing values fits in the next
            uint64_t chunk_mask = MK_CHUNK_MASK(num_trailing);
            // CHECK: page_index + num_free + 1?
            // if so, return the start-of-page-sequence value index
            uint64_t page = vm->mem.heap.gc_marks[page_index + num_free + 1];
            if( (page & chunk_mask) == 0 ) {
                return page_index * num_bits_per_page;
            }
        } else if( num_free < 0 ) {
            // we are out of pages (fail)
            return -1;
        }
        // else, continue looking
        page_index += _MAX(num_free, 1);
    }

    return -1;
}

int heap_find_free_chunk(vm_t* vm, int val_count) {
    int num_bits_per_page = sizeof(uint64_t) * CHAR_BIT;
    if( val_count < num_bits_per_page ) {
        return heap_find_small_chunk(vm, val_count);
    } else {
        return heap_find_large_chunk(vm, val_count);
    }
}

array_t heap_array_alloc(vm_t* vm, int val_count) {

    int addr = heap_find_free_chunk(vm, val_count);
    int end_addr = addr + val_count;

    // run GC if we are out of memory
    if( addr < 0 || end_addr >= vm->mem.heap.size ) {
        heap_gc_collect(vm);
        addr = heap_find_free_chunk(vm, val_count);
        end_addr = addr + val_count;
    }

    // if GC did not free up enough memory we fail
    if( addr < 0 || end_addr >= vm->mem.heap.size ) {
        sh_log_error("VM heap: not enough free memory.\n");
        return (array_t) { 0 }; // null address makes this invalid
    }

    // mark whole pages
    int num_bits_per_page = sizeof(uint64_t) * CHAR_BIT;
    int num_pages = CALC_GC_MARK_U64_COUNT(val_count) - 1;
    int page = HEAP_TO_PAGE_INDEX(addr);
    for(int page_index = page; page_index < num_pages; page_index++) {
        assert(vm->mem.heap.gc_marks[page_index] == 0UL);
        vm->mem.heap.gc_marks[page_index] = 0xFFFFFFFFFFFFFFFFUL;
    }

    // mark sub-page bits
    int trail_addr = addr + (num_pages * num_bits_per_page);
    int trail_page = page + num_pages;
    int trail_bits = val_count - (num_pages * num_bits_per_page);
    int shift = HEAP_TO_BIT_INDEX(trail_addr);
    uint64_t chunk_mask = MK_CHUNK_MASK(trail_bits);
    // assert that we are not overwriting previous data
    assert((vm->mem.heap.gc_marks[trail_page] & (chunk_mask << shift)) == 0);
    vm->mem.heap.gc_marks[trail_page] |= (chunk_mask << shift);

    // set all values
    for(int i = 0; i < val_count; i++) {
        vm->mem.heap.values[addr + i] = (val_t){ 0 };
    }

    return (array_t) {
        MEM_MK_PROGR_ADDR(vm->mem.stack.size + addr),
        val_count
    };
}

int heap_array_copy_to(vm_t* vm, val_t* src, int length, array_t dest) {
    int dest_index = MEM_ADDR_TO_INDEX(dest.address);
    val_t* dest_ptr = vm->mem.membase + dest_index;
    int dest_length = (int) dest.length;
    int copy_length = ( dest_length < length )
        ? dest_length
        : length;
    for(int i = 0; i < copy_length; i++) {
        dest_ptr[i] = src[i];
    }
    return copy_length;
}

