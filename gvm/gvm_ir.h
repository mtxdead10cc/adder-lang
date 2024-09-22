#ifndef GVM_IR_H_
#define GVM_IR_H_

#include "gvm_types.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct ir_inst_t {
    gvm_op_t opcode;
    uint32_t args[2];
} ir_inst_t;

typedef struct ir_list_t {
    uint32_t count;
    uint32_t capacity;
    ir_inst_t* irs;
} ir_list_t;

typedef enum ir_index_tag_t {
    IRID_INVALID = 0,
    IRID_VAR,
    IRID_FUN,
    IRID_INS
} ir_index_tag_t;

typedef struct ir_index_t {
    ir_index_tag_t tag;
    uint32_t    idx;
} ir_index_t;

inline static bool irl_init(ir_list_t* list, uint32_t capacity) {
    list->count = 0;
    list->irs = (ir_inst_t*) malloc( sizeof(ir_inst_t) * capacity );
    if( list->irs == NULL ) {
        return false;
    }
    list->capacity = capacity;
    return true;
}

inline static void irl_destroy(ir_list_t* list) {
    if( list->irs != NULL ) {
        free(list->irs);
    }
    list->irs = NULL;
    list->capacity = 0;
    list->count = 0;
}

inline static bool irl_reserve(ir_list_t* list, uint32_t additional) {
    uint32_t required = (list->count + additional);
    if( list->capacity <= required ) {
        ir_inst_t* ptr = (ir_inst_t*) realloc(list->irs, sizeof(ir_inst_t) * required);
        if( ptr == NULL ) {
            return false;
        }
        list->irs = ptr;
    }
    return true;
}

inline static ir_index_t irl_add(ir_list_t* list, ir_inst_t instr) {
    if( irl_reserve(list, 1) == false ) {
        printf("error: out of memory, can't realloc.");
        return (ir_index_t) {0};
    }
    list->irs[list->count++] = instr;
    return (ir_index_t) {
        .idx = list->count - 1,
        .tag = IRID_INS
    };
}

inline static ir_inst_t* irl_get(ir_list_t* list, ir_index_t index) {
    assert(index.tag == IRID_INS && "index has mismatching tag");
    return list->irs + index.idx;
}

inline static void irl_dump(ir_list_t* list) {
    for(uint32_t i = 0; i < list->count; i++) {
        int argcount = au_get_op_instr_arg_count(list->irs[i].opcode);
        printf("%03d #  ('%s'", i, au_get_op_name(list->irs[i].opcode));
        for (int j = 0; j < argcount; j++) {
            printf(" %d", list->irs[i].args[j]);
        }
        printf(")\n");
    }
}

#endif // GVM_IR_H_