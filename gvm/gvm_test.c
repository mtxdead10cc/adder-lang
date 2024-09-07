#include "gvm_test.h"
#include "gvm_types.h"
#include "gvm.h"
#include "gvm_heap.h"
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

    vm.mem.stack.values[++vm.mem.stack.top] = heap_alloc_array(&vm, 70);
    heap_gc_collect(&vm);
    //heap_print_usage(&vm);

    vm.mem.stack.values[++vm.mem.stack.top] = heap_alloc_array(&vm, 70);
    heap_gc_collect(&vm);
    //heap_print_usage(&vm);

    vm.mem.stack.top--;
    heap_gc_collect(&vm);
    //heap_print_usage(&vm);

    vm.mem.stack.values[++vm.mem.stack.top] = heap_alloc_array(&vm, 5);
    heap_gc_collect(&vm);
    //heap_print_usage(&vm);

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