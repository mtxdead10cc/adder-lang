#include "test_runner.h"
#include <vm.h>
#include <vm_heap.h>
#include <sh_value.h>
#include <sh_arena.h>
#include <co_ast.h>
#include <co_trace.h>
#include <co_parser.h>
#include <co_compiler.h>
#include <co_program.h>
#include <co_bty.h>
#include <sh_program.h>
#include <sh_log.h>
#include <vm_env.h>
#include <sh_ffi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "termhax.h"
#include "langtest.h"
#include <sh_ift.h>
#include <xu_lib.h>
#include <xu_invoke.h>
#include <vm_value_tools.h>

typedef struct test_case_t test_case_t;

typedef void (*test_case_fn)(test_case_t* this);

typedef struct test_case_t {
    char* name;
    test_case_fn test;
    int nfailed;
} test_case_t;

#define TEST_ASSERT_MSG(TC, COND, ...) do {     \
    if(!(COND)) {                               \
        sh_log_error("  \u2193 test assert | " __VA_ARGS__); \
        (TC)->nfailed++;                        \
    }                                           \
} while(false)


void test_heap_memory(test_case_t* this) {
    vm_t vm;

    TEST_ASSERT_MSG(this, vm_create(&vm, 512), "failed to create gvm\n");

    vm.mem.stack.top = -1;

    array_t array = heap_array_alloc(&vm, 70);
    TEST_ASSERT_MSG(this,
        ADDR_IS_NULL(array.address) == false,
        "#1.0 (heap_array_alloc) failed");
    vm.mem.stack.values[++vm.mem.stack.top] = val_array(array);

    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#1.1 (heap_array_alloc) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#1.2 (heap_array_alloc) failed");

    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#2.1 (heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#2.2 (heap_gc_collect) failed");
    //heap_print_usage(&vm);

    array = heap_array_alloc(&vm, 70);
    TEST_ASSERT_MSG(this,
        ADDR_IS_NULL(array.address) == false,
        "#3.0 (heap_array_alloc) failed\n");
    vm.mem.stack.values[++vm.mem.stack.top] = val_array(array);
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#3.1 (heap_alloc_array & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#3.2 (heap_alloc_array & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0xFFFFFFFFFFFFFFFFUL,
        "#3.3 (heap_alloc_array & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x3FUL,
        "#3.4 (heap_alloc_array & heap_gc_collect) failed");
    //heap_print_usage(&vm);

    vm.mem.stack.top--;
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#4.1 (pop & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#4.2 (pop & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0x0UL,
        "#4.3 (pop & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x0UL,
        "#4.4 (pop & heap_gc_collect) failed");
    //heap_print_usage(&vm);

    array = heap_array_alloc(&vm, 2);
    TEST_ASSERT_MSG(this,
        ADDR_IS_NULL(array.address) == false,
        "#4.0 (heap_array_alloc) failed\n");
    vm.mem.stack.values[++vm.mem.stack.top] = val_array(array);
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#5.1 (heap_alloc_array & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0xFFUL,
        "#5.2 (heap_alloc_array & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0x0UL,
        "#5.3 (heap_alloc_array & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x0UL,
        "#5.4 (heap_alloc_array & heap_gc_collect) failed");
    //heap_print_usage(&vm);

    vm.mem.stack.top--;
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#6.1 (pop & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#6.2 (pop & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0x0UL,
        "#6.3 (pop & heap_gc_collect) failed");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x0UL,
        "#6.4 (pop & heap_gc_collect) failed");

    vm_destroy(&vm);
}

void test_utils(test_case_t* this) {

    srcref_t ref = srcref_const("[##hello##]");

    TEST_ASSERT_MSG(this,
        srcref_starts_with_string(ref, "[##"),
        "#1.1 srcref_starts_with_string failed");

    TEST_ASSERT_MSG(this,
        srcref_starts_with_string(ref, "[##?") == false,
        "#1.2 negated srcref_starts_with_string failed");

    TEST_ASSERT_MSG(this,
        srcref_ends_with_string(ref, "##]"),
        "#2.1 srcref_ends_with_string failed");

    TEST_ASSERT_MSG(this,
        srcref_ends_with_string(ref, "&##]") == false,
        "#2.2 negated srcref_ends_with_string failed");

    srcref_t a = srcref_trim_left(ref, 3);
    TEST_ASSERT_MSG(this,
        srcref_equals_string(a, "hello##]"),
        "#3.1 srcref_trim_left failed");

    srcref_t b = srcref_trim_right(a, 3);
    TEST_ASSERT_MSG(this,
        srcref_equals_string(b, "hello"),
        "#3.2 srcref_trim_right failed");

    srcref_t c = srcref_trim_right(b, 30);
    TEST_ASSERT_MSG(this,
        srcref_equals_string(c, ""),
        "#3.3 srcref_trim_right overflow failed");

    sstr_t s = sstr("");
    sstr_t sadd = sstr("ABC");
    int rem = sstr_append(&s, &sadd);

    TEST_ASSERT_MSG(this,
        rem == (SSTR_MAX_LEN - 3),
        "#4.1 sstr append remaining failed");

    TEST_ASSERT_MSG(this,
        sstr_equal_str(&s, "ABC"),
        "#4.2 sstr append failed");

    rem = sstr_append_nstr(&s, "EFGH", 1);
    
    TEST_ASSERT_MSG(this,
        rem == (SSTR_MAX_LEN - 4),
        "#4.3 sstr n-append remaining failed");

    TEST_ASSERT_MSG(this,
        sstr_equal_str(&s, "ABCE"),
        "#4.4 sstr n-append failed");

    rem = sstr_append_str(&s, "F");
    
    TEST_ASSERT_MSG(this,
        rem == (SSTR_MAX_LEN - 5),
        "#4.5 sstr append c-str remaining failed");

    TEST_ASSERT_MSG(this,
        sstr_equal_str(&s, "ABCEF"),
        "#4.6 sstr append c-str failed");

    rem = sstr_append_fmt(&s, "%.3f", 1.1111f);
    
    TEST_ASSERT_MSG(this,
        rem == (SSTR_MAX_LEN - 10),
        "#4.7 sstr append fmt remaining failed");

    TEST_ASSERT_MSG(this,
        sstr_equal_str(&s, "ABCEF1.111"),
        "#4.8 sstr append fmt failed");

    int count = (SSTR_MAX_LEN - 10);

    assert(SSTR_MAX_LEN <= 500 && "update this test");
    rem = sstr_append_fmt(&s, "%.*s",
        count,
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------"
        "--------------------");

    TEST_ASSERT_MSG(this,
        rem == 0,
        "#4.9 sstr append fmt remaining failed");

    int check_count = 0;
    for(int i = 0; i < SSTR_MAX_LEN; i++) {
        if( s.str[i] == '-' )
            check_count ++;
    }

    TEST_ASSERT_MSG(this,
        check_count == count,
        "#4.10 sstr append fmt remaining failed");
}


void test_vm(test_case_t* this) {

    vm_t vm;

    program_t program = { 0 };

    TEST_ASSERT_MSG(this,
        vm_create(&vm, 32),
        "#1.0 failed to create VM.");
    
    u8buffer_t instr_buf;
    TEST_ASSERT_MSG(this,
        u8buffer_create(&instr_buf, 1),
        "#1.1 failed to create instruction buffer");

    valbuffer_t const_buf;
    TEST_ASSERT_MSG(this,
        valbuffer_create(&const_buf, 1),
        "#1.2 failed to create const buffer");

    // TEST simple program: 100 - 3

    // PROGRAM -- BEGIN

    TEST_ASSERT_MSG(this,
        0 == valbuffer_insert_int(&const_buf, 100).index,
        "2.1 unexpected index.");

    TEST_ASSERT_MSG(this,
        1 == valbuffer_insert_int(&const_buf, 3).index,
        "2.2 unexpected index.");

    TEST_ASSERT_MSG(this,
        0 == valbuffer_insert_int(&const_buf, 100).index,
        "2.3 unexpected index.");

    uint32_t value;

    value = 1;
    u8buffer_write(&instr_buf, OP_PUSH_VALUE);
    u8buffer_write(&instr_buf,  value);
    u8buffer_write(&instr_buf, (value >> (8*1)));
    u8buffer_write(&instr_buf, (value >> (8*2)));
    u8buffer_write(&instr_buf, (value >> (8*3)));

    value = 0;
    u8buffer_write(&instr_buf, OP_PUSH_VALUE);
    u8buffer_write(&instr_buf,  value);
    u8buffer_write(&instr_buf, (value >> (8*1)));
    u8buffer_write(&instr_buf, (value >> (8*2)));
    u8buffer_write(&instr_buf, (value >> (8*3)));

    u8buffer_write(&instr_buf, OP_SUB);
    u8buffer_write(&instr_buf, OP_RETURN_VALUE);

    // PROGRAM -- END

    program.cons.buffer = const_buf.values;
    program.cons.count = const_buf.size;
    program.inst.size = instr_buf.size;
    program.inst.buffer = instr_buf.data;

    vm_env_t env = { 0 };
    vm_env_setup(&env, &program, NULL);

    entry_point_t ep = { 0 };
    program_entry_point_find_default(&program, &ep);

    TEST_ASSERT_MSG(this,
        vm_env_is_ready(&env),
        "#3.0 call check failed");

    val_t ret = vm_execute(&vm, &env, &ep, &program);

    TEST_ASSERT_MSG(this,
        ret.type == VAL_NUMBER,
        "#3.1 unexpected return type.");

    TEST_ASSERT_MSG(this,
        val_into_number(ret) == 97.0f,
        "#3.2 unexpected return value.");

    // TEST simple program: 100 + VM ARG

    valbuffer_clear(&const_buf);
    u8buffer_clear(&instr_buf);

    // PROGRAM -- BEGIN

    TEST_ASSERT_MSG(this,
        0 == valbuffer_insert_int(&const_buf, 100).index,
        "2.1 unexpected index.");

    value = 0;
    u8buffer_write(&instr_buf, OP_PUSH_VALUE);
    u8buffer_write(&instr_buf,  value);
    u8buffer_write(&instr_buf, (value >> (8*1)));
    u8buffer_write(&instr_buf, (value >> (8*2)));
    u8buffer_write(&instr_buf, (value >> (8*3)));

    u8buffer_write(&instr_buf, OP_ADD);
    u8buffer_write(&instr_buf, OP_RETURN_VALUE);

    // PROGRAM -- END

    vm_env_setup(&env, &program, NULL);

    program.cons.buffer = const_buf.values;
    program.cons.count = const_buf.size;
    program.inst.size = instr_buf.size;
    program.inst.buffer = instr_buf.data;

    TEST_ASSERT_MSG(this,
        vm_env_is_ready(&env),
        "#4.0 call check failed");

    program_entry_point_find_default(&program, &ep);
    program_entry_point_set_arg_unsafe(&ep, 0, val_number(10));

    ret = vm_execute(&vm, &env, &ep, &program);

    TEST_ASSERT_MSG(this,
        ret.type == VAL_NUMBER,
        "#4.1 unexpected return type.");

    TEST_ASSERT_MSG(this,
        val_into_number(ret) == 110.0f,
        "#4.2 unexpected return value.");

    vm_destroy(&vm);
    u8buffer_destroy(&instr_buf);
    valbuffer_destroy(&const_buf);
    vm_env_destroy(&env);
}

void test_ast(test_case_t* this) {
    char* buf = "mainABtmpI";

    arena_t* arena = arena_create(1024);

    ast_node_t* decl_args = ast_arglist(arena);

    ast_arglist_add(arena, decl_args,
        ast_tyannot(arena,
            ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT)),
            ast_varref(arena, 
                srcref(buf, 4, 1))));

    ast_arglist_add(arena, decl_args,
        ast_tyannot(arena,
            ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT)),
            ast_varref(arena, 
                srcref(buf, 5, 1))));

    ast_node_t* body = ast_block(arena);

    ast_block_add(arena, body, 
        ast_assign(arena, 
            ast_tyannot(arena,
                ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT)),
                    ast_varref(arena, 
                        srcref(buf, 6, 3))),
            ast_binop(arena, AST_BIN_ADD,
                ast_varref(arena, srcref(buf, 5, 1)),
                ast_varref(arena, srcref(buf, 4, 1)))));

    ast_block_add(arena, body,
        ast_if(arena, 
            ast_binop(arena, AST_BIN_LT,
                ast_varref(arena, srcref(buf, 6, 3)),
                ast_float(arena, 0.0f)),
            ast_return(arena, ast_float(arena, 0.0f)),
            ast_block(arena)));

    ast_node_t* array = ast_array(arena);
    ast_array_add(arena, array, ast_float(arena, 1));
    ast_array_add(arena, array, ast_float(arena, 1));
    ast_array_add(arena, array, ast_float(arena, 1));
    ast_array_add(arena, array, ast_float(arena, 1));

    ast_block_add(arena, body,
        ast_foreach(arena, 
            ast_tyannot(arena,
                ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT)),
                    ast_varref(arena, 
                        srcref(buf, 9, 1))),
            array,
            ast_assign(arena, 
                ast_varref(arena, srcref(buf, 6, 3)),
                ast_binop(arena, AST_BIN_ADD,
                    ast_varref(arena, srcref(buf, 6, 3)),
                    ast_varref(arena, srcref(buf, 9, 1))))
        ));
    
    ast_block_add(arena, body,
        ast_return(arena, ast_varref(arena, srcref(buf, 6, 3))));

    ast_node_t* fun = ast_tyannot(arena,
        ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT)),
        ast_exported_fundecl(arena,
            srcref(buf, 0, 4),
            decl_args,
            body));

    trace_t trace = { 0 };
    trace_init(&trace, 16);

    program_t program = gvm_compile(arena, ast_block_with(arena, fun), &trace);
    if( trace_get_error_count(&trace) > 0 ) {
        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        sh_log_error("COMPILER\n%s", str.ptr);
    }

    arena_destroy(arena);
    trace_destroy(&trace);

    vm_t vm;

    TEST_ASSERT_MSG(this,
        vm_create(&vm, 32),
        "#1.0 failed to create VM.");

    vm_env_t env = { 0 };
    vm_env_setup(&env, &program, NULL);

    entry_point_t ep = {0};
    
    program_entry_point_find_any(&program, "main", &ep);

    program_entry_point_set_arg(&ep, 0, val_number(1));
    program_entry_point_set_arg(&ep, 1, val_number(-1));
    
    val_t ret = vm_execute(&vm, &env, &ep, &program);

    TEST_ASSERT_MSG(this,
        ret.type == VAL_NUMBER,
        "#1.1 unexpected return type.");

    TEST_ASSERT_MSG(this,
        val_into_number(ret) == 4.0f,
        "#1.2 unexpected return value.");

    program_destroy(&program);
    vm_destroy(&vm);
    vm_env_destroy(&env);
}

typedef struct toktest_t {
    char*           text;
    token_type_t*   tokens_types;
    bool            incl_comments;
    bool            incl_space;
} toktest_t;

void test_tokenizer(test_case_t* this) {
    token_collection_t coll;
    tokens_init(&coll, 16);

    toktest_t subtests[] = {
        {
            .text = "hej\n1//test\n2",
            .tokens_types = (token_type_t[]){
                TT_INITIAL,
                TT_SYMBOL,
                TT_SPACE,
                TT_NUMBER,
                TT_COMMENT,
                TT_NUMBER,
                TT_FINAL
            },
            .incl_comments = true,
            .incl_space = true
        },
        {
            .text = "hej\n1.3 \t \n //test\n2.4567",
            .tokens_types = (token_type_t[]){
                TT_INITIAL,
                TT_SYMBOL,
                TT_SPACE,
                TT_NUMBER,
                TT_SPACE,
                TT_COMMENT,
                TT_NUMBER,
                TT_FINAL
            },
            .incl_comments = true,
            .incl_space = true
        },
        {
            .text = "hej_325_sdg true False false//test234325",
            .tokens_types = (token_type_t[]){
                TT_INITIAL,
                TT_SYMBOL,
                TT_SPACE,
                TT_BOOLEAN,
                TT_SPACE,
                TT_SYMBOL,
                TT_SPACE,
                TT_BOOLEAN,
                TT_FINAL
            },
            .incl_comments = false,
            .incl_space = true
        },
        {
            .text = "(3.5) true\n//false\n/\"hej\"",
            .tokens_types = (token_type_t[]){
                TT_INITIAL,
                TT_OPEN_PAREN,
                TT_NUMBER,
                TT_CLOSE_PAREN,
                TT_BOOLEAN,
                TT_BINOP_DIV,
                TT_STRING,
                TT_FINAL
            },
            .incl_comments = false,
            .incl_space = false
        },
        {
            .text = "func_name(1,2,3,\"hej\",false)",
            .tokens_types = (token_type_t[]){
                TT_INITIAL,
                TT_SYMBOL,
                TT_OPEN_PAREN,
                TT_NUMBER,
                TT_SEPARATOR,
                TT_NUMBER,
                TT_SEPARATOR,
                TT_NUMBER,
                TT_SEPARATOR,
                TT_STRING,
                TT_SEPARATOR,
                TT_BOOLEAN,
                TT_CLOSE_PAREN,
                TT_FINAL
            },
            .incl_comments = false,
            .incl_space = false
        },
        {
            .text = "(1+2*3-4%5)",
            .tokens_types = (token_type_t[]){
                TT_INITIAL,
                TT_OPEN_PAREN,
                TT_NUMBER,
                TT_BINOP_PLUS,
                TT_NUMBER,
                TT_BINOP_MUL,
                TT_NUMBER,
                TT_BINOP_MINUS,
                TT_NUMBER,
                TT_BINOP_MOD,
                TT_NUMBER,
                TT_CLOSE_PAREN,
                TT_FINAL
            },
            .incl_comments = false,
            .incl_space = false
        },
        {
            .text = "a = b;//\n//",
            .tokens_types = (token_type_t[]){
                TT_INITIAL,
                TT_SYMBOL,
                TT_ASSIGN,
                TT_SYMBOL,
                TT_STATEMENT_END,
                TT_FINAL
            },
            .incl_comments = false,
            .incl_space = false
        }
    };

    size_t nsubcases = sizeof(subtests) / sizeof(subtests[0]);

    trace_t trace = {0};

    trace_init(&trace, 16);

    for(size_t i = 0; i < nsubcases; i++) {
        
        tokenizer_args_t args = (tokenizer_args_t) {
            .filepath = "test/test/test.txt",
            .include_comments = subtests[i].incl_comments,
            .include_spaces = subtests[i].incl_space,
            .text = subtests[i].text,
            .text_length = strlen(subtests[i].text),
            .trace = &trace
        };

        trace_clear(&trace);

        tokens_clear(&coll);
        tokenizer_analyze(&coll, &args);

        //tokens_print(&coll);
        
        size_t token_index = 0;
        while( token_index < coll.count ) {
            token_type_t expected = subtests[i].tokens_types[token_index];
            token_type_t check = coll.tokens[token_index].type;
            TEST_ASSERT_MSG(this,
                expected == check,
                "token #%d: expected(%s) != check(%s)",
                    (unsigned int) token_index,
                    token_get_type_name(expected),
                    token_get_type_name(check));
            token_index ++;
            if( expected == TT_FINAL ) {
                break;
            }
        }

        TEST_ASSERT_MSG(this,
            coll.count == token_index,
            "token count: expected(%d) != check(%d)",
                (unsigned int) token_index,
                (unsigned int) coll.count);

        tokens_destroy(&coll);

        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        if( strlen(str.ptr) > 0 )
            sh_log_error("%s", str.ptr);
    }

    trace_destroy(&trace);
}

void test_printfn(ffi_hndl_meta_t md, int argcount, val_t* args) {
    (void)(argcount);
    define_cstr(str, 512);
    vm_sprint_val(str, md.vm, args[0]);
    sh_log("> %s", str.ptr);
}

#define TEST_MSG(COND, ...) do {   \
    if((COND) == false ) {                                       \
        sh_log_info("  \u2193 test todo assert | " __VA_ARGS__); \
    }                                                            \
} while(false)

bool test_setup_default_env(ffi_t* ffi) {
    bool res = ffi_init(ffi);
    if( res == false ) {
        sh_log_error("failed to init FFI.\n");
        return false;;
    }
    res = ffi_native_exports_define(&ffi->supplied,
        sstr("print"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_HOST_ACTION,
            .u.host_action = test_printfn,
        },
        ift_func_1(ift_void(),
            ift_list(ift_char())));
    if( res == false ) {
        sh_log_error("failed to register FFI function: print\n");
        return false;
    }
    return true;
}


bool test_compile_and_run(test_case_t* this, char* test_category, char* source_code, char* expected_result, char* tc_name, char* tc_filepath) {

    static char result_as_text[512] = {0};
    arena_t* arena = arena_create(1024);
    parser_t parser;
    trace_t trace;
    ffi_t ffi = { 0 };

    trace_init(&trace, 16);

    TEST_ASSERT_MSG(this,
        test_setup_default_env(&ffi),
        "failed to setup FFI");

    bool is_known_todo = strcmp(test_category, "todo") == 0;

    pa_result_t result = pa_init(&parser, arena, &trace, source_code, strlen(source_code), tc_filepath);

    if( is_known_todo ) {
        TEST_MSG(par_is_error(result) == false,
            "'%s': failed to initialize parser.", tc_name);
    } else {
        TEST_ASSERT_MSG(this,
            par_is_error(result) == false,
            "'%s': failed to initialize parser.", tc_name);
    }
    
    result = pa_parse_program(&parser);

    bool parsing_ok = par_is_node(result);

    if( is_known_todo ) {
        TEST_MSG(parsing_ok,
            "failed to parse test-program '%s:%s'.",
            tc_filepath, tc_name);
    } else {
        TEST_ASSERT_MSG(this,
            parsing_ok,
            "failed to parse test-program '%s:%s'.",
            tc_filepath, tc_name);
    }

    if( parsing_ok == false ) {
        if( is_known_todo == false ) {
            define_cstr(str, 2048);
            cstr_append_fmt(str, "[trace]\n");
            trace_sprint(str, &trace);
            cstr_append_fmt(str, "[tokens]\n");
            tokens_sprint(str, &parser.collection);
            sh_log_error("PARSER\n%s", str.ptr);
        }
        arena_destroy(arena);
        pa_destroy(&parser);
        trace_destroy(&trace);
        return is_known_todo;
    }

    ast_node_t* node = par_extract_node(result);

    program_t program = gvm_compile(arena, node, &trace);
    if( trace_get_error_count(&trace) > 0 && is_known_todo == false ) {
        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        sh_log_error("COMPILER\n%s", str.ptr);
        ast_dump(node);
    }

    if( is_known_todo ) {
        TEST_MSG(program.inst.size > 0,
            "'%s': failed to compile program.",
            tc_name);
    } else {
        TEST_ASSERT_MSG(this,
            program.inst.size > 0,
            "'%s': failed to compile program.",
            tc_name);
    }

    if( program.inst.size == 0 ) {
        pa_destroy(&parser);
        arena_destroy(arena);
        trace_destroy(&trace);
        ffi_destroy(&ffi);
        return is_known_todo;
    }

    vm_t vm;
    vm_create(&vm, 100);

    vm_env_t env = { 0 };
    vm_env_setup(&env, &program, &ffi);

    TEST_ASSERT_MSG(this,
        vm_env_setup(&env, &program, &ffi),
        "'%s': failed set up env.",
                tc_name);

    entry_point_t ep = {0};
    program_entry_point_find_any(&program, "main", &ep);

    val_t res = val_none();
    if( vm_env_is_ready(&env) ) {
        res = vm_execute(&vm, &env, &ep, &program);
    }

    // TODO: FIX VALUE PRINTING AT SOME POINT!

    switch(res.type) {
        case VAL_ARRAY: {
            int wlen = vm_get_string(&vm, res, result_as_text, sizeof(result_as_text));
            result_as_text[wlen] = '\0';
        } break;
        case VAL_BOOL: {
            if( val_into_bool(res) ) {
                sprintf(result_as_text, "true");
            } else {
                sprintf(result_as_text, "false");
            }
        } break;
        case VAL_NUMBER: {
            sprintf(result_as_text, "%f", val_into_number(res));
        } break;
        case VAL_CHAR: {
            sprintf(result_as_text, "%c", val_into_char(res));
        } break;
        case VAL_IVEC2: {
            ivec2_t v = val_into_ivec2(res);
            sprintf(result_as_text, "(%i, %i)", v.x, v.y);
        } break;
        case VAL_NONE: {
            sprintf(result_as_text, "<none>");
        }
        default: break;
    }

    bool match_ok = strncmp(result_as_text,
        expected_result,
        strlen(expected_result)) == 0;

    if( is_known_todo ) {
        TEST_MSG(match_ok,
            "'%s': unexpected result, expected '%s', got '%s'.",
            tc_name, expected_result, result_as_text);
    } else {
        TEST_ASSERT_MSG(this,
            match_ok,
            "'%s': unexpected result, expected '%s', got '%s'.",
            tc_name, expected_result, result_as_text);
    }
    
    if( match_ok == false && is_known_todo == false ) {
        ast_dump(node);
        program_disassemble(&program);
    }

    program_destroy(&program);
    pa_destroy(&parser);
    arena_destroy(arena);
    vm_destroy(&vm);
    trace_destroy(&trace);
    ffi_destroy(&ffi);
    vm_env_destroy(&env);

    return true;
}


void test_langtest(test_case_t* this) {
    
    char* text = 
    "int main() {\n"
    "  int q = 0;\n"
    "  for(int y in [1,2,3,4,5]) {\n"
    "      q = q + (y * 5);\n"
    "  }\n"
    "  return q;\n"
    "}\n";

    bool accepted = test_compile_and_run(this,
        "verify",
        text, "75.0",
        "initial: simple-main",
        "builtin");
    if( accepted == false )
        return; 

    size_t tc_count = sizeof(langtest_testcases) / sizeof(langtest_testcases[0]);
    for (size_t i = 0; i < tc_count; i++) {
        ltc_t tc = langtest_testcases[i];
        accepted = test_compile_and_run(this,
            tc.category,
            tc.code,
            tc.expect,
            tc.name,
            tc.filepath);
        if( accepted == false )
            return; 
    }
}

void test_arena_alloc(test_case_t* this) {

    arena_t* a = arena_create(sizeof(int));
    int* nums = (int*) aalloc(a, sizeof(int)*10);
    for(int i = 0; i < 10; i++) {
        nums[i] = i + 1;
    }

    bool all_match = true;
    for(int i = 0; i < 10; i++) {
        all_match = all_match && (nums[i] == i + 1);
    }

    TEST_ASSERT_MSG(this,
        all_match,
        "#1.1 arena int array");

    nums = arealloc(a, nums, sizeof(int)*20);
    for(int i = 9; i < 20; i++) {
        nums[i] = i + 1;
    }

    all_match = true;
    for(int i = 0; i < 20; i++) {
        all_match = all_match && (nums[i] == i + 1);
    }

    TEST_ASSERT_MSG(this,
            all_match,
            "#1.2 arena int array after realloc");

    nums = arealloc(a, nums, sizeof(int)*20);

    all_match = true;
    for(int i = 0; i < 20; i++) {
        all_match = all_match && (nums[i] == i + 1);
    }

    TEST_ASSERT_MSG(this,
        all_match,
        "#1.3 arena int array after realloc");

    nums = arealloc(a, nums, sizeof(int)*20);

    all_match = true;
    for(int i = 0; i < 20; i++) {
        all_match = all_match && (nums[i] == i + 1);
    }

    TEST_ASSERT_MSG(this,
        all_match,
        "#1.3 arena int array after realloc");

    nums = arealloc(a, nums, sizeof(int) * 5);

    all_match = true;
    for(int i = 0; i < 5; i++) {
        all_match = all_match && (nums[i] == i + 1);
    }

    TEST_ASSERT_MSG(this,
        all_match,
        "#1.4 arena int array after realloc");

    nums = arealloc(a, nums, sizeof(int) * 500);

    int acc = 0;
    for(int i = 0; i < 500; i++) {
       acc += nums[i];
    }

    TEST_ASSERT_MSG(this,
        (acc == 15),
        "#1.5 arena int array after realloc");

    aalloc(a, 100);

    // arena_dump(a);

    arena_destroy(a);
}

bool check_ctx_lookup(bty_ctx_t* ctx, const char* name, bty_tag_t expected) {
    bty_type_t* res = bty_ctx_lookup(ctx, srcref_const(name));
    if( res == NULL )
        return false;
    return res->tag == expected;
}

void test_typing_context(test_case_t* this) {
    arena_t* a = arena_create(2048);

    trace_t trace = { 0 };
    trace_init(&trace, 5);

    bty_ctx_t* c = bty_ctx_create(a, &trace, 1);

    TEST_ASSERT_MSG(this,
        bty_ctx_insert(c, srcref_const("a"), bty_int()),
        "#1.1 bty_ctx_insert");

    TEST_ASSERT_MSG(this,
        bty_ctx_insert(c, srcref_const("sdaga"), bty_bool()),
        "#1.2 bty_ctx_insert");

    TEST_ASSERT_MSG(this,
        bty_ctx_insert(c, srcref_const("0sdgfg"), bty_float()),
        "#1.3 bty_ctx_insert");

    TEST_ASSERT_MSG(this,
        check_ctx_lookup(c, "a", BTY_INT),
        "#2.1 bty_ctx_lookup");

    TEST_ASSERT_MSG(this,
        check_ctx_lookup(c, "sdaga", BTY_BOOL),
        "#2.2 bty_ctx_lookup");

    TEST_ASSERT_MSG(this,
        check_ctx_lookup(c, "0sdgfg", BTY_FLOAT),
        "#2.3 bty_ctx_lookup");

    c = bty_ctx_clone(c);
    
    TEST_ASSERT_MSG(this,
        check_ctx_lookup(c, "a", BTY_INT),
        "#3.1 bty_ctx_lookup");

    TEST_ASSERT_MSG(this,
        check_ctx_lookup(c, "sdaga", BTY_BOOL),
        "#3.2 bty_ctx_lookup");

    TEST_ASSERT_MSG(this,
        check_ctx_lookup(c, "0sdgfg", BTY_FLOAT),
        "#3.3 bty_ctx_lookup");

    trace_destroy(&trace);
    arena_destroy(a);
}

void test_inference(test_case_t* this) {
    arena_t* a = arena_create(2048);

    trace_t trace = { 0 };
    trace_init(&trace, 5);

    ast_annot_t* arr_annot = ast_annot(a, srcref_const("array"));
    ast_annot_add_child(a, arr_annot, ast_annot(a, srcref_const("float")));

    ast_node_t* arr = ast_array(a);
    ast_array_add(a, arr, ast_int(a, 1));
    ast_array_add(a, arr, ast_int(a, 2));
    ast_array_add(a, arr, ast_int(a, 3));
    ast_array_add(a, arr, ast_int(a, 4));
    ast_array_add(a, arr, ast_int(a, 5));

    ast_node_t* n = ast_assign(a,
        ast_tyannot(a,
            arr_annot,
            ast_varref(a, 
                srcref_const("var"))),
        arr);

    bty_ctx_t* ctx = bty_ctx_create(a, &trace, 16);
    bty_synthesize(ctx, n);

    //sh_log_info("Type: %s\n", sprint_bty_type(a, t));

    TEST_ASSERT_MSG(this,
        trace_get_error_count(&trace) == 0,
        "#1.0 synth error");

    //bty_ctx_dump(ctx);
    trace_clear(&trace);

    ctx = bty_ctx_create(a, &trace, 16);
    
    n = ast_binop(a,
        AST_BIN_OR,
        ast_binop(a, AST_BIN_AND,
            ast_bool(a, false),
            ast_bool(a, true)),
        ast_binop(a, AST_BIN_EQ,
            ast_int(a, 0),
            ast_int(a, 1)));

    bty_synthesize(ctx, n);

    //sh_log_info("Type: %s\n", sprint_bty_type(a, t));

    TEST_ASSERT_MSG(this,
        trace_get_error_count(&trace) == 0,
        "#1.1 synth error");

    if( trace_get_error_count(&trace) > 0 ) {
        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        sh_log_error("TYPECHECKING\n%s", str.ptr);
    }

    trace_destroy(&trace);
    arena_destroy(a);
}

void test_ift_types(test_case_t* this) {

    ift_t t = ift_unknown();
    TEST_ASSERT_MSG(this, t.count == 1 && t.tags[0] == IFT_UNK, "#1.1 ift type error");
    t = ift_void();
    TEST_ASSERT_MSG(this, t.count == 1 && t.tags[0] == IFT_VOID, "#1.2 ift type error");
    t = ift_bool();
    TEST_ASSERT_MSG(this, t.count == 1 && t.tags[0] == IFT_BOOL, "#1.3 ift type error");
    t = ift_char();
    TEST_ASSERT_MSG(this, t.count == 1 && t.tags[0] == IFT_CHAR, "#1.4 ift type error");
    t = ift_int();
    TEST_ASSERT_MSG(this, t.count == 1 && t.tags[0] == IFT_I32, "#1.5 ift type error");
    t = ift_float();
    TEST_ASSERT_MSG(this, t.count == 1 && t.tags[0] == IFT_F32, "#1.6 ift type error");

    t = ift_func(ift_bool());
    TEST_ASSERT_MSG(this,
        t.count == 3
        && t.tags[0] == IFT_FUN
        && t.tags[1] == IFT_BOOL
        && t.tags[2] == IFT_ENDFUN,
        "#2.1 ift func type error");

    TEST_ASSERT_MSG(this,
        ift_func_arg_count(t) == 0,
        "#2.2 ift func type error");

    TEST_ASSERT_MSG(this,
        ift_func_get_arg(t, 0).tags[0] == IFT_UNK,
        "#2.3 ift func type error");

    t = ift_func_add_arg(t, ift_float());
    t = ift_func_add_arg(t, ift_float());
    t = ift_func_add_arg(t, ift_float());
    t = ift_func_add_arg(t, ift_char());

    TEST_ASSERT_MSG(this,
        ift_func_arg_count(t) == 4,
        "#2.4 ift func type error");

    TEST_ASSERT_MSG(this,
        ift_func_get_arg(t, 0).tags[0] == IFT_F32,
        "#2.5 ift func type error");
    
    TEST_ASSERT_MSG(this,
        ift_func_get_arg(t, 1).tags[0] == IFT_F32,
        "#2.6 ift func type error");
    
    TEST_ASSERT_MSG(this,
        ift_func_get_arg(t, 2).tags[0] == IFT_F32,
        "#2.7 ift func type error");

    TEST_ASSERT_MSG(this,
        ift_func_get_arg(t, 3).tags[0] == IFT_CHAR,
        "#2.8 ift func type error");

    TEST_ASSERT_MSG(this,
        ift_func_get_return_type(t).tags[0] == IFT_BOOL,
        "#2.9 ift func type error");

    t = ift_list(ift_int());

    TEST_ASSERT_MSG(this,
        t.tags[0] == IFT_LST && t.tags[1] == IFT_I32,
        "#3.1 ift list type error");

    TEST_ASSERT_MSG(this,
        ift_list_get_content_type(t).tags[0] == IFT_I32,
        "#3.2 ift list type error");

    t = ift_list(ift_list(ift_list(ift_int())));
    TEST_ASSERT_MSG(this,
        ift_list_get_content_type(t).tags[0] == IFT_LST,
        "#3.3 ift list type error");

    t = ift_list_get_content_type(t);
    t = ift_list_get_content_type(t);
    t = ift_list_get_content_type(t);

    TEST_ASSERT_MSG(this,
        t.tags[0] == IFT_I32,
        "#3.4 ift list type error");

    t = ift_list_get_content_type(t);
    TEST_ASSERT_MSG(this,
        t.tags[0] == IFT_UNK,
        "#3.4 ift list type error");

    ift_t f = ift_func(ift_int());
    f = ift_func_add_arg(f, ift_int());

    t = ift_list(f);
    t = ift_list_get_content_type(t);

    TEST_ASSERT_MSG(this,
        t.count == 4
        && t.tags[0] == IFT_FUN
        && t.tags[1] == IFT_I32
        && t.tags[2] == IFT_I32
        && t.tags[3] == IFT_ENDFUN,
        "#3.5 ift list type error");

    TEST_ASSERT_MSG(this,
        ift_type_equals(&t, &f),
        "#4.0 ift type error");

    f = ift_func(ift_bool());
    TEST_ASSERT_MSG(this,
        ift_type_equals(&t, &f) == false,
        "#4.1 ift type error");

}

void test_xu_classes(test_case_t* this) {
    char* src_class = 
    "export int A() {\n"
    "   return 0;\n"
    "}\n"
    "export bool B(int v) {\n" 
    "   return (v%2) == 1;\n" 
    "}\n";
    
    xu_classlist_t list = {0};
    source_code_t code = program_source_from_memory(src_class, strlen(src_class));
    xu_class_t class = xu_class_create(&list, &code, 0xBEEF);

    TEST_ASSERT_MSG(this,
        xu_class_is_compiled(class),
        "#1.1 class");
    
    program_source_free(&code);

    xu_caller_t A = xu_class_extract(class, "A", ift_func(ift_int()));
    xu_caller_t B = xu_class_extract(class, "B", ift_func_1(ift_bool(), ift_int()));

    TEST_ASSERT_MSG(this,
        xu_class_caller_is_valid(A),
        "#2.1 entry point A");

    TEST_ASSERT_MSG(this,
        xu_class_caller_is_valid(B),
        "#2.1 entry point B");

    TEST_ASSERT_MSG(this,
        xu_finalize_all(&list),
        "#3.1 finalize");

    vm_t vm = {0};
    vm_create(&vm, 16);

    TEST_ASSERT_MSG(this,
        bcall(&vm, &A) == 0,
        "#4.1 call A");

    TEST_ASSERT_MSG(this,
        bcalli(&vm, &B, 0) == false,
        "#4.2 call B #1");

    TEST_ASSERT_MSG(this,
        bcalli(&vm, &B, 11) == true,
        "#4.2 call B #2");

    vm_destroy(&vm);
    xu_cleanup_all(&list);
}

val_t test_alloc(ffi_hndl_meta_t md, int argcount, val_t* args) {
    assert(argcount == 1);
    (void)(argcount);
    int n = val_into_number(args[0]);
    array_t a = heap_array_alloc(md.vm, n);
    val_t* ptr = array_get_ptr(md.vm, a, 0);
    for(int i = 0; i < n; i++) {
        ptr[i] = val_number(i + 1);
    }
    return val_array(a);
}

void test_vm_full_heap(test_case_t* this) {
    
    char* str = 
    "import array<int> alloc(int n);\n"
    "int main(int n) {\n" 
    "   array<int> arr = alloc(n);\n"  
    "   return 0;\n"
    "}\n";

    ffi_t ffi = { 0 };
    ffi_init(&ffi);

    ffi_native_exports_define(&ffi.supplied,
        sstr("alloc"),
        (ffi_handle_t) {
            .local = NULL,
            .tag = FFI_HNDL_HOST_FUNCTION,
            .u.host_function = test_alloc
        }, ift_func_1(ift_list(ift_int()), ift_int()));

    source_code_t code = program_source_from_memory(str, strlen(str));
    program_t program = program_compile(&code, false);
    program_source_free(&code);

    if( program_is_valid(&program) == false ) {
        TEST_ASSERT_MSG(this,
            false,
            "#2.0 failed to compile test program");
        ffi_destroy(&ffi);
        return;
    }

    entry_point_t ep = {0};
    
    program_entry_point_find(&program, "main", ift_func_1(ift_int(), ift_int()), &ep);
    
    if( program_entry_point_is_valid(ep) == false ) {
        TEST_ASSERT_MSG(this,
            false,
            "#2.1 failed access entry point");
        ffi_destroy(&ffi);
        program_destroy(&program);
        return;
    }

    program_entry_point_set_arg(&ep, 0, val_number(39));

    vm_t vm = (vm_t) {0};
    vm_create(&vm, 2*40);

    vm_env_t env = (vm_env_t) {0};
    vm_env_setup(&env, &program, &ffi);

    for(int i = 0; i < 100; i++) {
        vm_execute(&vm, &env, &ep, &program);
    }

    vm_destroy(&vm);
    vm_env_destroy(&env);
    ffi_destroy(&ffi);
    program_destroy(&program);
}


void test_vm_cleanup(test_case_t* this) {

    char* src_01 = 
    "array<int> SPAM() {\n"
    "   return [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13];\n"
    "}\n"
    "int main() {\n" 
    "   array<int> arr = SPAM();\n"
    "   int v = 0;\n"
    "   for(int i in arr) {\n"
    "       v = v + i;\n"
    "   }\n"
    "   return v;\n"  
    "}\n";

    source_code_t code = program_source_from_memory(src_01, strlen(src_01));
    program_t program = program_compile(&code, false);
    program_source_free(&code);

    if( program_is_valid(&program) == false ) {
        TEST_ASSERT_MSG(this,
            false,
            "#1.0 failed to compile test program");
        return;
    }

    entry_point_t ep = {0};
    program_entry_point_find(&program, "main", ift_func(ift_int()), &ep);
    if( program_entry_point_is_valid(ep) == false ) {
        TEST_ASSERT_MSG(this,
            false,
            "#1.1 failed access entry point");
        return;
    }

    vm_t vm = {0};
    vm_create(&vm, 40);

    vm_env_t env = {0};
    vm_env_setup(&env, &program, NULL);

    for(int i = 0; i < 100; i++) {
        vm_execute(&vm, &env, &ep, &program);
        heap_gc_collect(&vm);
        int heap_used = heap_get_used(&vm);
        TEST_ASSERT_MSG(this,
            heap_used == 0,
            "#1.2 heap");
        if(heap_used != 0)
            break;
    }

    vm_destroy(&vm);
    vm_env_destroy(&env);
    program_destroy(&program);
}


test_results_t run_testcases(void) {

    test_case_t test_cases[] = {
        {
            .name = "vm full heap",
            .test = test_vm_full_heap,
            .nfailed = 0
        },
        {
            .name = "vm cleanup",
            .test = test_vm_cleanup,
            .nfailed = 0
        },
        {
            .name = "ift types",
            .test = test_ift_types,
            .nfailed = 0
        },
        {
            .name = "sh alloc",
            .test = test_arena_alloc,
            .nfailed = 0
        },
        {
            .name = "vm heap",
            .test = test_heap_memory,
            .nfailed = 0
        },
        {
            .name = "virtual machine",
            .test = test_vm,
            .nfailed = 0
        },
        {
            .name = "co ast",
            .test = test_ast,
            .nfailed = 0
        },
        {
            .name = "co tokenizer",
            .test = test_tokenizer,
            .nfailed = 0
        },
        {
            .name = "utils test",
            .test = test_utils,
            .nfailed = 0
        },
        {
            .name = "typing context test",
            .test = test_typing_context,
            .nfailed = 0
        },
        {
            .name = "type inference test",
            .test = test_inference,
            .nfailed = 0
        },
        {
            .name = "language test",
            .test = test_langtest,
            .nfailed = 0
        },
        {
            .name = "xutils classes",
            .test = test_xu_classes,
            .nfailed = 0
        }
    };

    test_results_t result = { 0 };
    int test_case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    for(int i = 0; i < test_case_count; i++) {
        test_cases[i].test(&test_cases[i]);
        if( test_cases[i].nfailed != 0 ) {
            sh_log("\033[%dm%s\033[0m test case '%s'\n", 31, "FAILED", test_cases[i].name);
            result.nfailed ++;
        } else {
            sh_log("\033[%dm%s\033[0m test case '%s'\n", 32, "PASSED", test_cases[i].name);
            result.npassed ++;
        }
    }

    return result;
}
