#include "gvm_test.h"
#include "gvm_types.h"
#include "gvm.h"
#include "gvm_heap.h"
#include "gvm_value.h"
#include "gvm_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef struct test_case_t test_case_t;

typedef void (*test_case_fn)(test_case_t* this);

typedef struct test_case_t {
    char* name;
    test_case_fn test;
    int nfailed;
} test_case_t;

#define TEST_ASSERT_MSG(TC, COND, ...) do { if(!(COND)) { printf(__VA_ARGS__); (TC)->nfailed++; } } while(false)

void test_heap_memory(test_case_t* this) {
    gvm_t vm;

    TEST_ASSERT_MSG(this, gvm_create(&vm, 16, 256), "failed to create gvm\n");

    vm.mem.stack.top = -1;

    array_t array = heap_array_alloc(&vm, 70);
    TEST_ASSERT_MSG(this,
        ADDR_IS_NULL(array.address) == false,
        "#1.0 (heap_array_alloc) failed\n");
    vm.mem.stack.values[++vm.mem.stack.top] = val_array(array);

    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#1.1 (heap_array_alloc) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#1.2 (heap_array_alloc) failed\n");

    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#2.1 (heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#2.2 (heap_gc_collect) failed\n");
    //heap_print_usage(&vm);

    array = heap_array_alloc(&vm, 70);
    TEST_ASSERT_MSG(this,
        ADDR_IS_NULL(array.address) == false,
        "#3.0 (heap_array_alloc) failed\n");
    vm.mem.stack.values[++vm.mem.stack.top] = val_array(array);
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#3.1 (heap_alloc_array & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#3.2 (heap_alloc_array & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0xFFFFFFFFFFFFFFFFUL,
        "#3.3 (heap_alloc_array & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x3FUL,
        "#3.4 (heap_alloc_array & heap_gc_collect) failed\n");
    //heap_print_usage(&vm);

    vm.mem.stack.top--;
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#4.1 (pop & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#4.2 (pop & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0x0UL,
        "#4.3 (pop & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x0UL,
        "#4.4 (pop & heap_gc_collect) failed\n");
    //heap_print_usage(&vm);

    array = heap_array_alloc(&vm, 2);
    TEST_ASSERT_MSG(this,
        ADDR_IS_NULL(array.address) == false,
        "#4.0 (heap_array_alloc) failed\n");
    vm.mem.stack.values[++vm.mem.stack.top] = val_array(array);
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#5.1 (heap_alloc_array & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0xFFUL,
        "#5.2 (heap_alloc_array & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0x0UL,
        "#5.3 (heap_alloc_array & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x0UL,
        "#5.4 (heap_alloc_array & heap_gc_collect) failed\n");
    //heap_print_usage(&vm);

    vm.mem.stack.top--;
    heap_gc_collect(&vm);
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[0] == 0xFFFFFFFFFFFFFFFFUL,
        "#6.1 (pop & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[1] == 0x3FUL,
        "#6.2 (pop & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[2] == 0x0UL,
        "#6.3 (pop & heap_gc_collect) failed\n");
    TEST_ASSERT_MSG(this,
        vm.mem.heap.gc_marks[3] == 0x0UL,
        "#6.4 (pop & heap_gc_collect) failed\n");

    gvm_destroy(&vm);
}

test_results_t run_testcases() {

    test_case_t test_cases[] = {
        {
            .name = "gvm heap",
            .test = test_heap_memory,
            .nfailed = 0
        }
    };

    test_results_t result = { 0 };
    int test_case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    for(int i = 0; i < test_case_count; i++) {
        test_cases[i].test(&test_cases[i]);
        if( test_cases[i].nfailed != 0 ) {
            printf("  test case '%s' [FAILED]\n", test_cases[i].name);
            result.nfailed ++;
        } else {
            printf("  test case '%s' [PASSED]\n", test_cases[i].name);
            result.npassed ++;
        }
    }

    return result;
}