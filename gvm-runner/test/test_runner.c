#include "test_runner.h"
#include <gvm_types.h>
#include <gvm.h>
#include <gvm_heap.h>
#include <gvm_value.h>
#include <gvm_memory.h>
#include <gvm_asmutils.h>
#include <gvm_ast.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "termhax.h"
#include <gvm_compiler.h>

typedef struct test_case_t test_case_t;

typedef void (*test_case_fn)(test_case_t* this);

typedef struct test_case_t {
    char* name;
    test_case_fn test;
    int nfailed;
} test_case_t;

#define TEST_ASSERT_MSG(TC, COND, ...) do { if(!(COND)) {  printf("  \u2193 test assert | "); printf(__VA_ARGS__); (TC)->nfailed++; printf("\n"); } } while(false)

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
        0 == au_consts_add_number(&const_buf, 100),
        "2.1 unexpected index.");

    TEST_ASSERT_MSG(this,
        1 == au_consts_add_number(&const_buf, 3),
        "2.2 unexpected index.");

    au_write_instr(&instr_buf, OP_PUSH_VALUE, 1);
    au_write_instr(&instr_buf, OP_PUSH_VALUE, 0);
    au_write_instr(&instr_buf, OP_SUB);
    au_write_instr(&instr_buf, OP_RETURN);

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
        0 == au_consts_add_number(&const_buf, 100),
        "2.1 unexpected index.");

    au_write_instr(&instr_buf, OP_PUSH_VALUE, 0);
    au_write_instr(&instr_buf, OP_ADD);
    au_write_instr(&instr_buf, OP_RETURN);

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

    ast_node_t* decl_args = ast_block();

    ast_block_add(decl_args,
        ast_vardecl(
            srcref(buf, 4, 1),
            AST_VALUE_TYPE_NUMBER));

    ast_block_add(decl_args,
        ast_vardecl(
            srcref(buf, 5, 1),
            AST_VALUE_TYPE_NUMBER));

    ast_node_t* body = ast_block();

    ast_block_add(body, 
        ast_assign(
            ast_vardecl(srcref(buf, 6, 3), AST_VALUE_TYPE_NUMBER),
            ast_binop(AST_BIN_ADD,
                ast_varref(srcref(buf, 5, 1)),
                ast_varref(srcref(buf, 4, 1))
            )));

    ast_block_add(body,
        ast_if(
            ast_binop(AST_BIN_LT,
                ast_varref(srcref(buf, 6, 3)),
                ast_number(0.0f)),
            ast_return(ast_number(0.0f))));

    ast_node_t* array = ast_array();
    ast_array_add(array, ast_number(1));
    ast_array_add(array, ast_number(1));
    ast_array_add(array, ast_number(1));
    ast_array_add(array, ast_number(1));

    ast_block_add(body,
        ast_foreach(
            ast_vardecl(
                srcref(buf, 9, 1),
                AST_VALUE_TYPE_NUMBER),
            array,
            ast_assign(
                ast_varref(srcref(buf, 6, 3)),
                ast_binop(AST_BIN_ADD,
                    ast_varref(srcref(buf, 6, 3)),
                    ast_varref(srcref(buf, 9, 1))))
        ));
    
    ast_block_add(body,
        ast_return(ast_varref(srcref(buf, 6, 3))));

    ast_node_t* fun = ast_fundecl(
        srcref(buf, 0, 4),
        AST_VALUE_TYPE_NUMBER,
        decl_args, body);

    // ast_dump(fun);

    gvm_program_t program = gvm_compile(fun);

    ast_free(fun);

    // gvm_program_disassemble(&program);

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

test_results_t run_testcases() {

    test_case_t test_cases[] = {
        {
            .name = "gvm heap",
            .test = test_heap_memory,
            .nfailed = 0
        },
        {
            .name = "gvm virtual machine",
            .test = test_vm,
            .nfailed = 0
        },
        {
            .name = "gvm ast",
            .test = test_ast,
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