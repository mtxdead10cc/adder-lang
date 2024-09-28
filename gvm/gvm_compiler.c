#include "gvm_compiler.h"
#include <assert.h>
#include "gvm_asminfo.h"

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
    IRID_INS
} ir_index_tag_t;

typedef struct ir_index_t {
    ir_index_tag_t tag;
    uint32_t    idx;
} ir_index_t;

bool irl_init(ir_list_t* list, uint32_t capacity) {
    list->count = 0;
    list->irs = (ir_inst_t*) malloc( sizeof(ir_inst_t) * capacity );
    if( list->irs == NULL ) {
        return false;
    }
    list->capacity = capacity;
    return true;
}

void irl_destroy(ir_list_t* list) {
    if( list->irs != NULL ) {
        free(list->irs);
    }
    list->irs = NULL;
    list->capacity = 0;
    list->count = 0;
}

bool irl_reserve(ir_list_t* list, uint32_t additional) {
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

ir_index_t irl_add(ir_list_t* list, ir_inst_t instr) {
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

ir_inst_t* irl_get(ir_list_t* list, ir_index_t index) {
    assert(index.tag == IRID_INS && "index has mismatching tag");
    return list->irs + index.idx;
}

void irl_dump(ir_list_t* list) {
    for(uint32_t i = 0; i < list->count; i++) {
        int argcount = get_op_arg_count(list->irs[i].opcode);
        printf("%03d #  ('%s'", i, get_op_name(list->irs[i].opcode));
        for (int j = 0; j < argcount; j++) {
            printf(" %d", list->irs[i].args[j]);
        }
        printf(")\n");
    }
}

typedef struct src2idxmap_t {
    size_t          count;
    size_t          capacity;
    ir_index_tag_t  tag;
    srcref_t*       key;
    bool*           is_in_use;
    ir_index_t*     value;
} src2idxmap_t;

void s2sim_destroy(src2idxmap_t* map) {
    if( map == NULL ) {
        return;
    }
    if( map->is_in_use != NULL ) {
        free(map->is_in_use);
        map->is_in_use = NULL;
    }
    if( map->key != NULL ) {
        free(map->key);
        map->key = NULL;
    }
    if( map->value != NULL ) {
        free(map->value);
        map->value = NULL;
    }
    map->capacity = 0;
    map->count = 0;
    map->tag = IRID_INVALID;
}

bool s2sim_init(src2idxmap_t* map, ir_index_tag_t tag, size_t initial_capacity) {
    map->capacity = initial_capacity;
    map->count = 0;
    map->tag = tag;
    map->is_in_use = (bool*) malloc(sizeof(bool) * map->capacity);
    if( map->is_in_use == NULL ) {
        s2sim_destroy(map);
        return false;
    }
    memset(map->is_in_use, 0, sizeof(bool) * map->capacity); // in_use = fale
    map->key = (srcref_t*) malloc(sizeof(srcref_t) * map->capacity);
    if( map->key == NULL ) {
        s2sim_destroy(map);
        return false;
    }
    map->value = (ir_index_t*) malloc(sizeof(ir_index_t) * map->capacity);
    if( map->value == NULL ) {
        s2sim_destroy(map);
        return false;
    }
    return true;
}

bool s2sim_ensure_capacity(src2idxmap_t* map, size_t additional) {
    // this is a map and not a list so we try to
    // have some headroom.
    size_t required = (map->count + additional); 
    if( map->capacity <= (required + (required / 4)) ) {
        size_t new_capacity = required * 2;
        bool* is_in_use = (bool*) realloc(map->is_in_use, sizeof(bool) * new_capacity);
        if( is_in_use != NULL ) {
            map->is_in_use = is_in_use;
        }
        srcref_t* key = (srcref_t*) realloc(map->key, sizeof(bool) * new_capacity);
        if( is_in_use != NULL ) {
            map->key = key;
        }
        ir_index_t* value = (ir_index_t*) realloc(map->value, sizeof(bool) * new_capacity);
        if( is_in_use != NULL ) {
            map->value = value;
        }
        if( (key == NULL) || (value == NULL) | (is_in_use == NULL) ) {
            printf("error: out of memory\n");
            return false;
        }
        map->capacity = new_capacity;
    }
    return true;
}



size_t s2im_hash(srcref_t ref) {
    size_t len = srcref_len(ref);
    size_t hash_code = len + 5;
    char* keystr = srcref_ptr(ref); 
    for(size_t i = 0; i < len; i++) {
        hash_code += (hash_code + keystr[i]) * 7919U;
    }
    return hash_code;
}

bool s2im_insert(src2idxmap_t* map, srcref_t key, ir_index_t val) {
    assert(val.tag == map->tag && "table tag and value mismatch");
    if( s2sim_ensure_capacity(map, 1) == false ) {
        return false;
    }
    size_t hk = s2im_hash(key);
    size_t start_index = hk % map->capacity;
    for(size_t i = 0; i < map->capacity; i++) {
        size_t tab_index = (i + start_index) % map->capacity;
        if( map->is_in_use[tab_index] == false ) {
            map->value[tab_index] = val;
            map->is_in_use[tab_index] = true;
            map->key[tab_index] = key;
            map->count ++;
            return true;
        } else if ( srcref_equals(map->key[tab_index], key) ) {
            return false;
        }
    }
    return false;
}

void s2im_clear(src2idxmap_t* map) {
    ir_index_tag_t tag = map->tag;
    memset(map->is_in_use, 0, sizeof(bool) * map->capacity);
    map->tag = tag;
    map->count = 0;
}

void s2im_print(src2idxmap_t* map) {
    printf("[Source -> Index Map (size=%d)]\n", (uint32_t) map->count);
    for(size_t i = 0; i < map->capacity; i++) {
        printf("%i > ", (uint32_t) i);
        if( map->is_in_use[i] == false ) {
            printf("<empty>");
        } else {
            srcref_print(map->key[i]);
        }
        printf("\n");
    }
}

ir_index_t* s2im_lookup(src2idxmap_t* map, srcref_t key) {
    size_t hk = s2im_hash(key);
    size_t start_index = hk % map->capacity;
    for(size_t i = 0; i < map->capacity; i++) {
        size_t tab_index = (i + start_index) % map->capacity;
        if( map->is_in_use[tab_index] == false ) {
            return NULL;
        }
        if( srcref_equals(map->key[tab_index], key) ) {
            return &map->value[tab_index];
        }
    }
    return NULL;
}

typedef struct compiler_state_t {
    src2idxmap_t     localvars;
    src2idxmap_t     functions;
    ir_list_t        instrs;
    valbuffer_t      consts;
} compiler_state_t;

void codegen(ast_node_t* node, compiler_state_t* state);

void codegen_binop(ast_binop_t node, compiler_state_t* state) {
    codegen(node.right, state);
    codegen(node.left, state);
    switch(node.type) {
        case AST_BIN_ADD: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_ADD,
                .args = { 0 }
            });
        } break;
        case AST_BIN_SUB: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_SUB,
                .args = { 0 }
            });
        } break;
        case AST_BIN_MUL: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_MUL,
                .args = { 0 }
            });
        } break;
        case AST_BIN_AND: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_AND,
                .args = { 0 }
            });
        } break;
        case AST_BIN_OR: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_OR,
                .args = { 0 }
            });
        } break;
        case AST_BIN_EQ: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_EQUAL,
                .args = { 0 }
            });
        } break;
        case AST_BIN_LT: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_LESS_THAN,
                .args = { 0 }
            });
        } break;
        case AST_BIN_GT: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_MORE_THAN,
                .args = { 0 }
            });
        } break;
        default: {
            printf("Unhandled binary operation: %s\n",
                ast_binop_type_as_string(node.type));
        } break;
    }
}

void codegen_unop(ast_unop_t node, compiler_state_t* state) {
    codegen(node.inner, state);
    switch(node.type) {
        case AST_UN_NEG: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_NEG,
                .args = { 0 }
            });
        } break;
        default: {
            printf("Unhandled unary operation: %s\n",
                ast_unop_type_as_string(node.type));
        } break;
    }
}

void codegen_value(ast_value_t node, compiler_state_t* state) {
    uint32_t const_index;
    switch(node.type) {
        case AST_VALUE_TYPE_BOOL: {
            const_index = valbuffer_add_bool(&state->consts,
                node.u._bool);
        } break;
        case AST_VALUE_TYPE_NUMBER: {
            const_index = valbuffer_add_number(&state->consts,
                node.u._number);
        } break;
        case AST_VALUE_TYPE_STRING: {
            // Note: needs to start with a '"'
            srcref_t ref = node.u._string;
            char* ptr = srcref_ptr(ref);
            assert(ptr[0] == '\"' && "current impl requres quoted strings.");
            const_index = valbuffer_add_string(&state->consts, ptr);
        } break;
        default: {
            /* ignore */
            printf("error: cant push constant reference, unsupported value type %d\n",
                node.type);
            assert(false && "unsupported constant value");
        } break;
    }
    
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_PUSH_VALUE,
        .args = { const_index, 0 }
    });
}

void codegen_fundecl(ast_fundecl_t node, compiler_state_t* state) {
    assert(node.args->type == AST_BLOCK);
    assert(state->localvars.count == 0 && "Function declared inside function?");
    ir_index_t frame_index = irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_MAKE_FRAME,
        .args = { 0 }
    });
    bool ok = s2im_insert(&state->functions, node.name, frame_index);
    assert(ok == true);
    s2im_clear(&state->localvars);
    codegen(node.args, state); // in order to "add" arg names
    uint32_t arg_count = (uint32_t) state->localvars.count;
    codegen(node.body, state); // adds locals to frame
    uint32_t locals_count = ((uint32_t) state->localvars.count) - arg_count;
    irl_get(&state->instrs, frame_index)->args[0] = arg_count;
    irl_get(&state->instrs, frame_index)->args[1] = locals_count;
    s2im_clear(&state->localvars);
}

void codegen_funcall(ast_funcall_t node, compiler_state_t* state) {
    codegen(node.args, state);
    ir_index_t* ir_index = s2im_lookup(&state->functions, node.name);
    assert(ir_index == NULL && "Function not found (not declared)");
    // if tag invalid: could not find index of function name (not defined)
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_CALL,
        .args = { ir_index->idx, 0 }
    });
}

void codegen_assignment(ast_assign_t node, compiler_state_t* state) {
    codegen(node.right_value, state);
    assert(state->localvars.count > 0 && "local vars was empty");
    srcref_t varname = { 0 };
    // depending on declared variable or just
    // a reference to already existing
    // we do slightly differen things.
    if( node.left_var->type == AST_VAR_DECL ) {
        codegen(node.left_var, state); // add var to known locals
        varname = node.left_var->u.n_vardecl.name;
    } else if ( node.left_var->type == AST_VAR_REF ) {
        varname = node.left_var->u.n_varref.name;
    } else {
        assert(false && "expected variable node as LSH in assignment");
    }
    ir_index_t* index = s2im_lookup(&state->localvars, varname);
    assert(index != NULL && "varname not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { (uint32_t) index->idx, 0 }
    });
}

void codegen_foreach(ast_foreach_t node, compiler_state_t* state) {
    codegen(node.collection, state);
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_MAKE_ITER,
        .args = { 0 }
    });
    ir_index_t loop_start_index = irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_ITER_NEXT,
        .args = { 0 }
    });
    codegen(node.vardecl, state); // add varname
    srcref_t varname = node.vardecl->u.n_vardecl.name;
    ir_index_t* varindex = s2im_lookup(&state->localvars, varname);
    assert(varindex != NULL && "variable not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { varindex->idx, 0 }
    });
    codegen(node.during, state);
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_JUMP,
        .args = { loop_start_index.idx, 0 }
    });
    irl_get(&state->instrs, loop_start_index)->args[0] = state->instrs.count;
}

void codegen(ast_node_t* node, compiler_state_t* state) {
    switch(node->type) {
        case AST_BINOP: {
            codegen_binop(node->u.n_binop, state);
        } break;
        case AST_UNOP: {
            codegen_unop(node->u.n_unop, state);
        } break;
        case AST_ASSIGN: {
            codegen_assignment(node->u.n_assign, state);
        } break;
        case AST_ARRAY: {
            size_t count = node->u.n_array.count;
            for(size_t i = 0; i < count; i++) {
                codegen(node->u.n_array.content[i], state);
            }
            uint32_t const_index = valbuffer_add_number(&state->consts, (float)count);
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_PUSH_VALUE,
                .args = { (uint32_t) const_index, 0 }
            });
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_MAKE_ARRAY,
                .args = { 0 }
            });
        } break;
        case AST_RETURN: {
            codegen(node->u.n_return.result, state);
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_RETURN,
                .args = { 0 }
            });
        } break;
        case AST_BLOCK: {
            size_t count = node->u.n_block.count;
            for(size_t i = 0; i < count; i++) {
                codegen(node->u.n_block.content[i], state);
            }
        } break;
        case AST_IF: {
            codegen(node->u.n_if.cond, state);
            ir_index_t if_index = irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_JUMP_IF_FALSE,
                .args = { 0 }
            });
            codegen(node->u.n_if.iftrue, state);
            irl_get(&state->instrs, if_index)->args[0] = state->instrs.count;
        } break;
        case AST_IF_ELSE: {
            codegen(node->u.n_ifelse.cond, state);
            ir_index_t if_false_index = irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_JUMP_IF_FALSE,
                .args = { 0 }
            });
            codegen(node->u.n_ifelse.iftrue, state);
            ir_index_t jump_end_index = irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_JUMP,
                .args = { 0 }
            });
            // set jump loc for if false
            irl_get(&state->instrs, if_false_index)->args[0] = state->instrs.count;
            codegen(node->u.n_ifelse.iffalse, state);
            irl_get(&state->instrs, jump_end_index)->args[0] = state->instrs.count;
        } break;
        case AST_FOREACH: {
            codegen_foreach(node->u.n_foreach, state);
        } break;
        case AST_FUN_DECL: {
            codegen_fundecl(node->u.n_fundecl, state);
        } break;
        case AST_FUN_CALL: {
            codegen_funcall(node->u.n_funcall, state);
        } break;
        case AST_VAR_REF: {
            ir_index_t* var_index = s2im_lookup(&state->localvars, node->u.n_varref.name);
            assert(var_index != NULL && "variable not found");
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_LOAD_LOCAL,
                .args = { var_index->idx, 0 }
            });
        } break;
        case AST_VALUE: {
            codegen_value(node->u.n_value, state);
        } break;
        case AST_VAR_DECL: {
            // just add valiable name to frame local var set.
            s2im_insert(&state->localvars, node->u.n_vardecl.name, (ir_index_t){
                .tag = IRID_VAR,
                .idx = (uint32_t) state->localvars.count
            });
        } break;
        case AST_BREAK: {
            assert(false && "break op is not implemented yet");
        } break;
    }
}

void recalc_index_to_bytecode_adress(ir_list_t* instrs) {
    
    uint32_t idx2addr[instrs->count];
    uint32_t addr = 0;
    const uint32_t argbytes = 4; // 32-bit args
    
    for (uint32_t i = 0; i < instrs->count; i++) {
        idx2addr[i] = addr;
        uint32_t argcount = get_op_arg_count(instrs->irs[i].opcode);
        addr = addr + (argcount * argbytes) + 1;
    }

    for (uint32_t i = 0; i < instrs->count; i++) {
        switch(instrs->irs[i].opcode) {
            case OP_CALL:
            case OP_ENTRY_POINT:
            case OP_ITER_NEXT:
            case OP_JUMP:
            case OP_JUMP_IF_FALSE: {
                uint32_t index = instrs->irs[i].args[0];
                instrs->irs[i].args[0] = idx2addr[index];
            } break;
            default: break;
        }
    }
}

gvm_program_t write_program(ir_list_t* instrs, valbuffer_t* consts) {

    recalc_index_to_bytecode_adress(instrs);

    u8buffer_t bytecode;
    u8buffer_create(&bytecode, instrs->count);

    for (uint32_t i = 0; i < instrs->count; i++) {
        u8buffer_write(&bytecode, (uint8_t) instrs->irs[i].opcode);
        uint32_t argcount = get_op_arg_count(instrs->irs[i].opcode);
        for (uint32_t j = 0; j < argcount; j++) {
            uint32_t value = instrs->irs[i].args[j];
            u8buffer_write(&bytecode, (uint8_t) (value & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*1)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*2)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*3)) & 0xFF));
        }
    }

    val_t* const_buf = (val_t*) malloc( sizeof(val_t) * consts->size );
    memcpy(const_buf, consts->values, sizeof(val_t) * consts->size);

    uint8_t* code_buf = (uint8_t*) malloc( sizeof(uint8_t) * bytecode.size );
    memcpy(code_buf, bytecode.data, sizeof(uint8_t) * bytecode.size );

    gvm_program_t result = (gvm_program_t) {
        .cons.buffer = const_buf,
        .cons.count = consts->size,
        .inst.buffer = code_buf,
        .inst.size = bytecode.size
    };

    u8buffer_destroy(&bytecode);
    return result;
}


gvm_program_t gvm_compile(ast_node_t* node) {

    compiler_state_t state = (compiler_state_t) { 0 };

    if( s2sim_init(&state.functions, IRID_INS, 16) == false ) {
        return (gvm_program_t) {0};
    }
    
    if( s2sim_init(&state.localvars, IRID_VAR, 16) == false ) {
        return (gvm_program_t) {0};
    }

    if( irl_init(&state.instrs, 16) == false ) {
        return (gvm_program_t) {0};
    }

    if( valbuffer_create(&state.consts, 16) == false ) {
        return (gvm_program_t) {0};
    }

    ir_index_t entrypoint = irl_add(&state.instrs, (ir_inst_t) {
        .opcode = OP_ENTRY_POINT,
        .args = { 0 }
    });

    codegen(node, &state);

    ir_index_t* index = s2im_lookup( &state.functions,
                                     srcref("main", 0, 4, NULL) );
    assert(index != NULL && "main entrypoint not found");
    irl_get(&state.instrs, entrypoint)->args[0] = index->idx;

    // irl_dump(&state.instrs);

    gvm_program_t program = write_program(&state.instrs, &state.consts);
    
    valbuffer_destroy(&state.consts);
    irl_destroy(&state.instrs);
    s2sim_destroy(&state.localvars);
    s2sim_destroy(&state.functions);

    return program;
}