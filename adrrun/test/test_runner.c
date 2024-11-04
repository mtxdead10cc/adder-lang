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
#include <co_typing.h>
#include <sh_program.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "termhax.h"
#include "langtest.h"

typedef struct test_case_t test_case_t;

typedef void (*test_case_fn)(test_case_t* this);

typedef struct test_case_t {
    char* name;
    test_case_fn test;
    int nfailed;
} test_case_t;

#define TEST_ASSERT_MSG(TC, COND, ...) do {     \
    if(!(COND)) {                               \
        printf("  \u2193 test assert | ");      \
        printf(__VA_ARGS__);                    \
        (TC)->nfailed++;                        \
        printf("\n");                           \
    }                                           \
} while(false)

void test_heap_memory(test_case_t* this) {
    gvm_t vm;

    TEST_ASSERT_MSG(this, gvm_create(&vm, 16, 256), "failed to create gvm\n");

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

    gvm_destroy(&vm);
}

void test_shared_utils(test_case_t* this) {

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
}

void test_vm(test_case_t* this) {

    gvm_t vm;

    gvm_program_t program = { 0 };

    gvm_exec_args_t args = {
        .args = { 0 },
        .cycle_limit = 10
    };

    TEST_ASSERT_MSG(this,
        gvm_create(&vm, 16, 16),
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

    val_t ret = gvm_execute(&vm, &program, &args);

    TEST_ASSERT_MSG(this,
        VAL_GET_TYPE(ret) == VAL_NUMBER,
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

    val_t num = val_number(10);
    args.args.buffer = &num;
    args.args.count = 1;

    program.cons.buffer = const_buf.values;
    program.cons.count = const_buf.size;
    program.inst.size = instr_buf.size;
    program.inst.buffer = instr_buf.data;

    ret = gvm_execute(&vm, &program, &args);

    TEST_ASSERT_MSG(this,
        VAL_GET_TYPE(ret) == VAL_NUMBER,
        "#4.1 unexpected return type.");

    TEST_ASSERT_MSG(this,
        val_into_number(ret) == 110.0f,
        "#4.2 unexpected return value.");

    gvm_destroy(&vm);
    u8buffer_destroy(&instr_buf);
    valbuffer_destroy(&const_buf);
}

void test_ast(test_case_t* this) {
    char* buf = "mainABtmpI";

    arena_t* arena = arena_create(1024);

    ast_node_t* decl_args = ast_block(arena);

    ast_block_add(arena, decl_args,
        ast_vardecl(arena, 
            srcref(buf, 4, 1),
            ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT))));

    ast_block_add(arena, decl_args,
        ast_vardecl(arena, 
            srcref(buf, 5, 1),
            ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT))));

    ast_node_t* body = ast_block(arena);

    ast_block_add(arena, body, 
        ast_assign(arena, 
            ast_vardecl(arena, srcref(buf, 6, 3), ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT))),
            ast_binop(arena, AST_BIN_ADD,
                ast_varref(arena, srcref(buf, 5, 1)),
                ast_varref(arena, srcref(buf, 4, 1))
            )));

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
            ast_vardecl(arena, 
                srcref(buf, 9, 1),
                ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT))),
            array,
            ast_assign(arena, 
                ast_varref(arena, srcref(buf, 6, 3)),
                ast_binop(arena, AST_BIN_ADD,
                    ast_varref(arena, srcref(buf, 6, 3)),
                    ast_varref(arena, srcref(buf, 9, 1))))
        ));
    
    ast_block_add(arena, body,
        ast_return(arena, ast_varref(arena, srcref(buf, 6, 3))));

    ast_node_t* funsign = ast_funsign(arena, srcref(buf, 0, 4),
        decl_args, AST_FUNSIGN_INTERN,
        ast_annot(arena, srcref_const(LANG_TYPENAME_FLOAT)));

    ast_node_t* fun = ast_fundecl(arena, funsign, body);

    trace_t trace = { 0 };
    trace_init(&trace, 16);

    gvm_program_t program = gvm_compile(fun, &trace);
    if( trace_get_error_count(&trace) > 0 ) {
        trace_fprint(stdout, &trace);
    }

    arena_destroy(arena);
    trace_destroy(&trace);

    gvm_t vm;
    val_t argbuf[] = { val_number(1), val_number(-1) };
    gvm_exec_args_t args = {
        .args = { .buffer = argbuf, .count = 2 },
        .cycle_limit = 100
    };

    TEST_ASSERT_MSG(this,
        gvm_create(&vm, 16, 16),
        "#1.0 failed to create VM.");

    val_t ret = gvm_execute(&vm, &program, &args);
     TEST_ASSERT_MSG(this,
        VAL_GET_TYPE(ret) == VAL_NUMBER,
        "#1.1 unexpected return type.");

    TEST_ASSERT_MSG(this,
        val_into_number(ret) == 4.0f,
        "#1.2 unexpected return value.");

    gvm_program_destroy(&program);
    gvm_destroy(&vm);
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
            .text = "a = b;",
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

        trace_fprint(stdout, &trace);
    }

    trace_destroy(&trace);
}

val_t test_printfn(gvm_t* vm, size_t argcount, val_t* args) {
    (void)(argcount);
    printf(" >    ");
    gvm_print_val(vm, args[0]);
    return val_none();
}

#define TEST_MSG(COND, ...) do {   \
    if((COND) == false ) {                              \
        printf("  \u2193 test todo assert | ");         \
        printf(__VA_ARGS__);                            \
        printf("\n");                                   \
    }                                                   \
} while(false)

void test_compile_and_run(test_case_t* this, char* test_category, char* source_code, char* expected_result, char* tc_name, char* tc_filepath) {

    static char result_as_text[512] = {0};
    arena_t* arena = arena_create(1024);
    parser_t parser;
    trace_t trace;

    trace_init(&trace, 16);

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
            trace_fprint(stdout, &trace);
            tokens_print(&parser.collection);
        }
        arena_destroy(arena);
        pa_destroy(&parser);
        trace_destroy(&trace);
        return;
    }

    ast_node_t* node = par_extract_node(result);

    typecheck(arena, &trace, node);
    if( trace_get_error_count(&trace) > 0 && is_known_todo == false ) {
        trace_fprint(stdout, &trace);
        trace_clear(&trace);
    }

    gvm_program_t program = gvm_compile(node, &trace);
    if( trace_get_error_count(&trace) > 0 && is_known_todo == false ) {
        trace_fprint(stdout, &trace);
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
        return;
    }

    gvm_t vm;
    gvm_create(&vm, 50, 50);
    gvm_exec_args_t args = {
        .args = { 0 },
        .cycle_limit = 100
    };

    gvm_native_func(&vm, "print", "type", 1, &test_printfn);

    val_t res = gvm_execute(&vm, &program, &args);

    // TODO: FIX VALUE PRINTING AT SOME POINT!

    switch(VAL_GET_TYPE(res)) {
        case VAL_ARRAY: {
            int wlen = gvm_get_string(&vm, res, result_as_text, sizeof(result_as_text));
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
        gvm_program_disassemble(stdout, &program);
    }

    gvm_program_destroy(&program);
    pa_destroy(&parser);
    arena_destroy(arena);
    gvm_destroy(&vm);
    trace_destroy(&trace);
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

    test_compile_and_run(this,
        "verify",
        text, "75.0",
        "simple-main",
        "builtin"); 

    size_t tc_count = sizeof(langtest_testcases) / sizeof(langtest_testcases[0]);
    for (size_t i = 0; i < tc_count; i++) {
        ltc_t tc = langtest_testcases[i];
        test_compile_and_run(this,
            tc.category,
            tc.code,
            tc.expect,
            tc.name,
            tc.filepath);
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

    nums = arealloc(a, nums, sizeof(int)*5);

    all_match = true;
    for(int i = 0; i < 5; i++) {
        all_match = all_match && (nums[i] == i + 1);
    }

    TEST_ASSERT_MSG(this,
        all_match,
        "#1.4 arena int array after realloc");

    nums = arealloc(a, nums, sizeof(int)*500);

    int acc = 0;
    for(int i = 0; i < 500; i++) {
       acc += nums[i];
    }

    TEST_ASSERT_MSG(this,
        (acc == 15),
        "#1.5 arena int array after realloc");

    aalloc(a, 100);

    arena_dump(a);

    arena_destroy(a);
}

void test_typing(test_case_t* this) {
    arena_t* a = arena_create(1024);

    ast_node_t* n = ast_block(a);
    ast_block_add(a, n, ast_int(a, 1));
    ast_block_add(a, n, ast_float(a, 1.0f));
    ast_block_add(a, n, ast_bool(a, false));
    ast_block_add(a, n, ast_char(a, 'h'));
    
    char* sign = make_signature(a, n);
    TEST_ASSERT_MSG(this,
        strcmp(sign, "ifbc") == 0,
        "#1.1 value signatures");

    n = ast_array(a);
    ast_array_add(a, n, ast_int(a, 1));
    ast_array_add(a, n, ast_int(a, 1));
    ast_array_add(a, n, ast_int(a, 1));
    ast_array_add(a, n, ast_int(a, 1));
    ast_array_add(a, n, ast_int(a, 1));

    sign = make_signature(a, n);
    TEST_ASSERT_MSG(this,
        strcmp(sign, "[i]") == 0,
        "#1.2 array signature");

    n = ast_array(a);
    ast_array_add(a, n, ast_int(a, 1));
    ast_array_add(a, n, ast_int(a, 1));
    ast_array_add(a, n, ast_float(a, 1.0f));
    ast_array_add(a, n, ast_int(a, 1));
    ast_array_add(a, n, ast_int(a, 1));

    sign = make_signature(a, n);
    TEST_ASSERT_MSG(this,
        strcmp(sign, "[*]") == 0,
        "#1.3 array invalid signature");

   
    ast_node_t* args = ast_block(a);
    ast_block_add(a, args, ast_float(a, 1.0f));
    ast_block_add(a, args, ast_int(a, 1));
    n = ast_funcall(a, srcref_const("name-funcall"), args);

    sign = make_signature(a, n);
    TEST_ASSERT_MSG(this,
        strcmp(sign, "#name-funcall:fi") == 0,
        "#1.3 funcall signature");

    ast_node_t* funsign = ast_funsign(a,
        srcref_const("name-fundecl"),
        args,
        AST_FUNSIGN_INTERN,
        ast_annot(a, srcref_const(LANG_TYPENAME_INT)));
    n = ast_fundecl(a, funsign, ast_block(a));

    sign = make_signature(a, n);
    TEST_ASSERT_MSG(this,
        strcmp(sign, "#name-fundecl:fi") == 0,
        "#1.4 fundecl signature");

    n = ast_binop(a, AST_BIN_ADD, ast_char(a, 'C'), ast_int(a, 1));

    sign = make_signature(a, n);
    TEST_ASSERT_MSG(this,
        strcmp(sign, "#+:ci") == 0,
        "#1.5 binop signature");

    n = ast_unnop(a, AST_UN_NEG, ast_char(a, 'C'));

    sign = make_signature(a, n);
    TEST_ASSERT_MSG(this,
        strcmp(sign, "#-:c") == 0,
        "#1.6 unnop signature");

    ctx_t* ctx = ctx_create(a, 1);
    ctx_insert(ctx, "ab", "2");
    ctx_insert(ctx, "aba", "3");
    ctx_insert(ctx, "a", "1");
    
    TEST_ASSERT_MSG(this,
        ctx->size == 3,
        "#2.0 context setup");

    int nerrors = 0;
    for(size_t i = 0; i < ctx->size; i++) {
        nerrors += (atoi(ctx->kvps[i].val) != ((int)i + 1));
    }

    TEST_ASSERT_MSG(this,
        nerrors == 0,
        "#2.1 context insert");

    char* res = ctx_infer(ctx, "aba");
    TEST_ASSERT_MSG(this,
        strcmp(res, "3") == 0,
        "#2.3 context infer");

    ctx_t* ctx2 = ctx_clone(ctx);

    TEST_ASSERT_MSG(this,
        ctx2->size == 3,
        "#2.4 context clone");

    nerrors = 0;
    for(size_t i = 0; i < ctx2->size; i++) {
        nerrors += (atoi(ctx2->kvps[i].val) != ((int)i + 1));
    }

    TEST_ASSERT_MSG(this,
        nerrors == 0,
        "#2.5 context clone check");

    arena_destroy(a);
}


test_results_t run_testcases() {

    test_case_t test_cases[] = {
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
            .name = "language test",
            .test = test_langtest,
            .nfailed = 0
        },
        {
            .name = "typing test",
            .test = test_typing,
            .nfailed = 0
        },
        {
            .name = "shared utils test",
            .test = test_shared_utils,
            .nfailed = 0
        }
    };

    test_results_t result = { 0 };
    int test_case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    for(int i = 0; i < test_case_count; i++) {
        test_cases[i].test(&test_cases[i]);
        if( test_cases[i].nfailed != 0 ) {
            printf("[");
            termhax_print_color("FAILED", COL_FG_RED);
            printf("]");
            result.nfailed ++;
        } else {
            printf("[");
            termhax_print_color("PASSED", COL_FG_GREEN);
            printf("]");
            result.npassed ++;
        }
        printf(" test case '%s'\n", test_cases[i].name);
    }

    return result;
}