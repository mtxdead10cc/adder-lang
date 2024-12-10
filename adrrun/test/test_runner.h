#ifndef TEST_RUNNER_H_
#define TEST_RUNNER_H_

typedef struct test_results_t {
    int nfailed;
    int npassed;
} test_results_t;

test_results_t run_testcases();

#endif // TEST_RUNNER_H_
