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
    const char *name;
    test_func_t func;
    struct test_case *next;
} test_case_t;

static test_case_t *test_list_head = NULL;

#define REGISTER_TEST(test_name) \
    void test_name(void); \
    __attribute__((constructor)) static void register_##test_name(void) { \
        static test_case_t test = { #test_name, test_name, NULL }; \
        test.next = test_list_head; \
        test_list_head = &test; \
    }

#define ASSERT(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET " [%s:%d] %s\n", \
                __FILE__, __LINE__, #expr); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static inline int run_all_tests(void) {
    int count = 0;
    int passed = 0;
    test_case_t *t = test_list_head;

    while (t) {
        printf(COLOR_YELLOW "RUNNING" COLOR_RESET " %s\n", t->name);
        fflush(stdout);
        t->func();
        delete_all_swaps(); // Clean up after each test
        if (get_swapfile_count() != 0) {
            fprintf(stderr, COLOR_RED "FAIL" COLOR_RESET " [%s:%d] Swap files not cleaned up\n",
                    __FILE__, __LINE__);
            return EXIT_FAILURE;
        }
        printf(COLOR_GREEN "PASS" COLOR_RESET "    %s\n\n", t->name);
        passed++;
        count++;
        t = t->next;
    }

    printf("Summary: %d/%d tests passed\n", passed, count);
    return (passed == count) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif // TEST_FRAMEWORK_H
