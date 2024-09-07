#ifndef GVM_TEST_H_
#define GVM_TEST_H_

typedef struct test_results_t {
    int nfailed;
    int npassed;
} test_results_t;

test_results_t run_testcases();

#endif // GVM_TEST_H_