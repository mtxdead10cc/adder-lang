#ifndef GVM_VALUE_TOOLS_H_
#define GVM_VALUE_TOOLS_H_

#include "sh_types.h"
#include "vm_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

void val_print(val_t val);
void val_print_lookup(val_t val, addr_lookup_fn lookup, void* user);
int  val_get_string(val_t val, addr_lookup_fn lookup, void* user, char* dest, int dest_len);
void val_print_lookup_val_array(val_t* lookup_buffer, val_t val);
int  val_get_string_val_array(val_t* lookup_buffer, val_t val, char* dest, int dest_len);
char* val_get_type_name(val_type_t type);

inline static val_t* array_get_ptr(gvm_t* vm, array_t array, int index) {
    assert(ADDR_IS_NULL(array.address) == false);
    if( ADDR_IS_CONST(array.address) ) {
        return (vm->run.constants + MEM_ADDR_TO_INDEX(array.address) + index);
    } else {
        return (vm->mem.membase + MEM_ADDR_TO_INDEX(array.address) + index);
    }
}

inline static val_t array_get(gvm_t* vm, array_t array, int index) {
    return *array_get_ptr(vm, array, index);
}

inline static void array_set(gvm_t* vm, array_t array, int index, val_t value) {
    val_t* loc = array_get_ptr(vm, array, index);
    *loc = value;
}

#endif // GVM_VALUE_TOOLS_H_