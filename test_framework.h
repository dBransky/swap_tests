#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_util.h" // for make_swaps()

#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_YELLOW  "\033[1;33m"

typedef void (*test_func_t)(void);

typedef struct test_case {
    char *name;
    test_func_t func;
    struct test_case *next;
} test_case_t;

static test_case_t *test_list_head = NULL;
static test_case_t *perf_test_list_head = NULL; // For performance tests
static int current_test_failed = 0;  // Track current test failure state

#define REGISTER_TEST(test_name) \
    void test_name(void); \
    __attribute__((constructor)) static void register_##test_name(void) { \
        static test_case_t test = { #test_name, test_name, NULL }; \
        test.next = test_list_head; \
        test_list_head = &test; \
    }
#define REGISTER_PERF_TEST(test_name) \
    void test_name(void); \
    __attribute__((constructor)) static void register_perf_##test_name(void) { \
        static test_case_t test = { #test_name, test_name, NULL }; \
        test.next = perf_test_list_head; \
        perf_test_list_head = &test; \
    }

#define ASSERT(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET " [%s:%d] %s\n", \
                __FILE__, __LINE__, #expr); \
        current_test_failed = 1; \
    } \
} while (0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET " [%s:%d] %s == %s (%lx != %lx)\n", \
                __FILE__, __LINE__, #a, #b, (unsigned long)(a), (unsigned long)(b)); \
        current_test_failed = 1; \
    } \
} while (0)

#define ASSERT_NEQ(a, b) do { \
    if ((a) == (b)) { \
        fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET " [%s:%d] %s != %s (%lx == %lx)\n", \
                __FILE__, __LINE__, #a, #b, (unsigned long)(a), (unsigned long)(b)); \
        current_test_failed = 1; \
    } \
} while (0)
// should work on uint_64
#define ASSERT_ABOVE(a, b) do { \
    if ((a) <= (b)) { \
        fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET " [%s:%d] %s > %s (%llu <= %llu)\n", \
                __FILE__, __LINE__, #a, #b, (unsigned long long)(a), (unsigned long long)(b)); \
        current_test_failed = 1; \
    } \
} while (0)
#define ASSERT_BELOW(a, b) do { \
    if ((a) >= (b)) { \
        fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET " [%s:%d] %s < %s (%lu >= %lu)\n", \
                __FILE__, __LINE__, #a, #b, (unsigned long long)(a), (unsigned long long)(b)); \
        current_test_failed = 1; \
    } \
} while (0)

static inline int run_all_tests(int enable_traces) {
    int count = 0;
    int passed = 0;
    test_case_t *t = test_list_head;
    pid_t pid = 0;
    set_minimal_swapfile_num(disable_swaps()); // Disable all swaps before running tests
    
    while (t) {
        fprintf(stderr, COLOR_YELLOW "RUNNING" COLOR_RESET " %s\n", t->name);
        fflush(stdout);
        
        // Reset failure state for this test
        current_test_failed = 0;
        
        if (enable_traces) 
            pid = start_ftrace(); // Start ftrace if needed
            
        t->func();  
        
        set_minimal_swapfile_num(disable_swaps()); // Disable all swaps before running tests
        
        if (enable_traces) 
            stop_ftrace(t->name, pid); // Stop ftrace if needed
        
        // Check if test passed or failed
        if (current_test_failed) {
            fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET "    %s\n", t->name);
        } else {
            fprintf(stderr, COLOR_GREEN "PASS" COLOR_RESET "    %s\n", t->name);
            passed++;
        }
        
        count++;
        t = t->next;
    }

    fprintf(stderr, "Summary: %d/%d tests passed\n", passed, count);
    return (passed == count) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static inline void run_perf_tests(int enable_traces) {
    fprintf(stderr, "Running performance tests...\n");
    int count = 0;
    int passed = 0;
    test_case_t *t = perf_test_list_head;
    pid_t pid = 0;
    while (t) {
        fprintf(stderr, COLOR_YELLOW "RUNNING" COLOR_RESET " %s\n", t->name);
        fflush(stdout);
        
        // Reset failure state for this test
        current_test_failed = 0;
        if(enable_traces) {
            pid = start_ftrace(); // Start ftrace if needed
        }
        t->func();
        if(enable_traces) {
            stop_ftrace(t->name, pid); // Stop ftrace if needed
        }
        
        // Check if test passed or failed
        if (current_test_failed) {
            fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET "    %s\n", t->name);
        } else {
            fprintf(stderr, COLOR_GREEN "PASS" COLOR_RESET "    %s\n", t->name);
            passed++;
        }
        
        count++;
        t = t->next;
    }
    fprintf(stderr, "Performance Summary: %d/%d tests passed\n", passed, count);

}

#endif // TEST_FRAMEWORK_H