#ifndef GVM_VM_TYPES_H_
#define GVM_VM_TYPES_H_

#include "sh_types.h"

typedef struct grid_t {
    int width;
    int height;
    type_id_t* data;
} grid_t;

typedef struct gvm_exec_args_t {
    struct {
        uint32_t    count; 
        val_t*      buffer;
    } args;
    uint32_t  cycle_limit;
} gvm_exec_args_t;

typedef struct gvm_stack_t {
    val_t* values;  // pointer to the stack
    int top;        // the index of the top element on the stack
    int frame;      // the index of the current stack (call) frame
    int size;       // size of the stack (in val_t count)
} gvm_stack_t;

typedef struct gvm_heap_t {
    uint64_t*   gc_marks; // garbage collector (marking region)
    val_t*      values;   // pointer to heap memory region
    int         size;     // size of the heap memory (in val_t count)
} gvm_heap_t;

typedef struct gvm_mem_t {
    val_t*      membase;   // base pointer to the memory region (stack + heap)
    int         memsize;   // total size of stack + heap (in val_t count)
    gvm_stack_t stack;
    gvm_heap_t  heap;
} gvm_mem_t;

typedef struct gvm_t gvm_t;

typedef struct func_result_t {
    int arg_count;
    val_t value;
} func_result_t;

typedef val_t* (*addr_lookup_fn)(void* user, val_addr_t addr);
typedef val_t (*func_t)(gvm_t* gvm, val_t* args);

typedef struct tabval_t {
    int     argc;
    func_t  func;
} tabval_t;

typedef struct func_table_t {
    char      key[GVM_ENV_NFUNC_NAME_MAX_LEN][GVM_ENV_NFUNC_TABLE_SIZE];
    bool      is_in_use[GVM_ENV_NFUNC_TABLE_SIZE];
    tabval_t  value[GVM_ENV_NFUNC_TABLE_SIZE];
} func_table_t;

typedef struct env_t {
    gvm_t*       vm;
    func_table_t table;
} env_t;

typedef struct gvm_runtime_t {
    val_t*   constants;
    uint8_t* instructions;
    uint32_t pc;
} gvm_runtime_t;

typedef struct gvm_t {
    env_t           env;
    gvm_mem_t       mem;
    gvm_runtime_t   run;
    void*           validation; // validation data (NULL if no validation)
} gvm_t;

#endif // GVM_VM_TYPES_H_