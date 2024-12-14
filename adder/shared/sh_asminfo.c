#include "sh_asminfo.h"
#include "sh_types.h"
#include <assert.h>

static op_info_t opinfo[OP_OPCODE_COUNT] = {
    { "halt",               0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "and",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "or",                 0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "not",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "mul",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "div",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "mod",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "add",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "sub",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "neg",                0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "dup-1",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "dup-2",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "rot-2",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "cmp(==)",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "cmp(!=)",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "cmp(<)",             0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "cmp(>)",             0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "cmp(<=)",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "cmp(>=)",            0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "push-const",         1, { OP_ARG_CONSTANT, OP_ARG_NONE  }     },
    { "pop-1",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "pop-2",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "jump",               1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "jump-if-false",      1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "exit",               1, { OP_ARG_NUMERIC, OP_ARG_NONE  }      },
    { "call",               1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "make-frame",         2, { OP_ARG_NUMERIC, OP_ARG_NUMERIC }    },
    { "return-nothing",     0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "return-value",       0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "store-local",        1, { OP_ARG_NUMERIC, OP_ARG_NONE  }      },
    { "load-local",         1, { OP_ARG_NUMERIC, OP_ARG_NONE  }      },
    { "print",              0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "make-array",         0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "array-length",       0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "make-iter",          0, { OP_ARG_NONE, OP_ARG_NONE }          },
    { "iter-next",          1, { OP_ARG_ADDRESS, OP_ARG_NONE  }      },
    { "call-native",        1, { OP_ARG_CONSTANT, OP_ARG_NONE  }     }
};

#define _OP_CODE_COUNT_VALIDATION 37

char* get_op_name(vm_op_t op_code) {
    assert(_OP_CODE_COUNT_VALIDATION == OP_OPCODE_COUNT);
    return opinfo[op_code].op_name;
}

size_t get_op_arg_count(vm_op_t op_code) {
    assert(_OP_CODE_COUNT_VALIDATION == OP_OPCODE_COUNT);
    return opinfo[op_code].op_arg_count;
}

op_argtype_t* get_op_arg_types(vm_op_t op_code) {
    assert(_OP_CODE_COUNT_VALIDATION == OP_OPCODE_COUNT);
    return opinfo[op_code].op_arg_types;
}
