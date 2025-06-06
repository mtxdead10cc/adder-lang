#ifndef GVM_ASMINFO_H_
#define GVM_ASMINFO_H_

#include "sh_types.h"

#define OP_MAX_ARG_COUNT 2

typedef enum op_argtype_t {
    OP_ARG_NONE,
    OP_ARG_CONSTANT, // a reference to a constant
    OP_ARG_ADDRESS,  // an adress of an instruction (label)
    OP_ARG_NUMERIC   // a numeric parameter
} op_argtype_t;

typedef struct op_info_t {
    char*        op_name;
    size_t       op_arg_count;
    op_argtype_t op_arg_types[OP_MAX_ARG_COUNT];
} op_info_t;

char*         get_op_name(vm_op_t op_code);
size_t        get_op_arg_count(vm_op_t op_code);
op_argtype_t* get_op_arg_types(vm_op_t op_code);

#endif // GVM_ASMINFO_H_
