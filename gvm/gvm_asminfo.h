#ifndef GVM_ASMINFO_H_
#define GVM_ASMINFO_H_

#include "gvm_types.h"

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

static op_info_t opinfo[OP_OPCODE_COUNT] = {
    { "HALT",             0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "AND",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "OR",               0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "NOT",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "MUL",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "ADD",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "SUB",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "NEG",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "DUP_1",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "DUP_2",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "ROT_2",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "CMP_EQUAL",        0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "CMP_LESS_THAN",    0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "CMP_MORE_THAN",    0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "PUSH_VALUE",       1, { OP_ARG_CONSTANT, OP_ARG_NONE  }     },
    { "POP_1",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "POP_2",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "JUMP",             1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "JUMP_IF_FALSE",    1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "EXIT",             1, { OP_ARG_NUMERIC, OP_ARG_NONE  }      },
    { "CALL",             1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "ENTRY_POINT",      1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "MAKE_FRAME",       2, { OP_ARG_NUMERIC, OP_ARG_NUMERIC }    },
    { "RETURN",           0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "STORE_LOCAL",      1, { OP_ARG_NUMERIC, OP_ARG_NONE  }     },
    { "LOAD_LOCAL",       1, { OP_ARG_NUMERIC, OP_ARG_NONE  }     },
    { "PRINT",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "MAKE_ARRAY",       0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "ARRAY_LENGTH",     0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "MAKE_ITER",        0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "ITER_NEXT",        1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "CALL_NATIVE",      1, { OP_ARG_CONSTANT, OP_ARG_NONE  }     }
};

inline static char* get_op_name(gvm_op_t op_code) {
    return opinfo[op_code].op_name;
}

inline static size_t get_op_arg_count(gvm_op_t op_code) {
    return opinfo[op_code].op_arg_count;
}

inline static op_argtype_t* get_op_arg_types(gvm_op_t op_code) {
    return opinfo[op_code].op_arg_types;
}


#endif // GVM_ASMINFO_H_