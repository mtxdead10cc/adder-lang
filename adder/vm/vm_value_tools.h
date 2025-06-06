#ifndef VM_VALUE_TOOLS_H_
#define VM_VALUE_TOOLS_H_

#include "sh_types.h"
#include "vm_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

void val_sprint(cstr_t str, val_t val);
void val_sprint_lookup(cstr_t str, val_t val, addr_lookup_fn lookup, void* user);
int  val_get_string(val_t val, addr_lookup_fn lookup, void* user, char* dest, int dest_len);
char* val_get_type_name(val_type_t type);

inline static val_t* array_get_ptr(vm_t* vm, array_t array, int index) {
    if(ADDR_IS_NULL(array.address)) 
        return NULL;

    if( ADDR_IS_CONST(array.address) )
        return (vm->run.constants + MEM_ADDR_TO_INDEX(array.address) + index);
    else
        return (vm->mem.membase + MEM_ADDR_TO_INDEX(array.address) + index);
}

inline static val_t array_get(vm_t* vm, array_t array, int index) {
    return *array_get_ptr(vm, array, index);
}

inline static void array_set(vm_t* vm, array_t array, int index, val_t value) {
    val_t* loc = array_get_ptr(vm, array, index);
    *loc = value;
}

#endif // VM_VALUE_TOOLS_H_
