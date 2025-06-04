#include "test_framework.h"
#include "test_util.h" // for make_swaps()

REGISTER_TEST(test_available_swapfile);
REGISTER_TEST(test_folio_offset);
REGISTER_TEST(test_multiple_swapfiles);
REGISTER_TEST(test_vma_si_allcation);
REGISTER_TEST(test_stack_vma_offset);
REGISTER_TEST(test_stack_vma_enlarge);

void test_stack_vma_offset(void) {
    make_swaps(1, 0);
    char* stack = mmap(NULL, 4096 * 10, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    ASSERT(stack != MAP_FAILED);
    for (int i = 0; i < 10; i++) {
        swapout_page(stack + (i * 4096));
        ASSERT(get_swap_offset_from_page(stack + (i * 4096)) == 9-i);
        stack[i * 4096] = i;
    }
}
void test_stack_vma_enlarge(void) {
    make_swaps(1, 0);
    char* stack = mmap(NULL, 4096 * 10, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    ASSERT(stack != MAP_FAILED);
    for (int i = 0; i < 10; i++) {
        swapout_page(stack + (i * 4096));
        ASSERT(get_swap_offset_from_page(stack + (i * 4096)) == 9-i);
        stack[i * 4096] = i;
    }
    stack-= 4096; // Move the stack pointer down by one page
    *stack = 11;
    swapout_page(stack);
    ASSERT(get_swap_offset_from_page(stack) == 10);
    ASSERT(*stack == 11);
    for (int i = 1; i < 11; i++) {
        swapout_page(stack + (i * 4096));
        ASSERT(get_swap_offset_from_page(stack + (i * 4096)) == 10-i);
        ASSERT(stack[i * 4096] == i-1);
    }
}

void test_vma_si_allcation(void) {
    make_swaps(1, 0);
    char *addr = map_anon_region(4096 * 10);
    for (int i = 0; i < 10; i++) {
        ASSERT(vma_has_swap_info(addr + (i * 4096))==0);
    }
    swapout_page(addr);
    for (int i = 0; i < 10; i++) {
        ASSERT(vma_has_swap_info(addr + (i * 4096))=1);
    }
}
void test_available_swapfile(void) {
    ASSERT(get_swapfile_count() == 0);
    make_swaps(1, 0);
    ASSERT(get_swapfile_count() == 1);
    // mmap an anon region
    void *addr = map_anon_region(4096);
    ASSERT(addr != NULL);
    swapout_page(addr);
    ASSERT(get_swapfile_count() == 0);
}
void test_folio_offset(void) {
    make_swaps(1, 0);
    char *addr = map_anon_region(4096*10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * 4096));
        ASSERT(get_swap_offset_from_page(addr + (i * 4096)) == i);
        ASSERT(addr[i] == i);
    }
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * 4096));
    }
    for (int i = 0; i < 10; i++) {
        ASSERT(get_swap_offset_from_page(addr + (i * 4096)) == i);
        ASSERT(addr[i] == i);
    }
}
void test_multiple_swapfiles(void) {
    make_swaps(3, 0);
    ASSERT(get_swapfile_count() == 3);
    char *addr = map_anon_region(4096 * 10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * 4096));
        ASSERT(get_swap_offset_from_page(addr + (i * 4096)) == i % 3);
        ASSERT(addr[i] == i);
    }
    ASSERT(get_swapfile_count() == 2);
    addr = map_anon_region(4096 * 10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * 4096));
        ASSERT(get_swap_offset_from_page(addr + (i * 4096)) == i % 3);
        ASSERT(addr[i] == i);
    }
    ASSERT(get_swapfile_count() == 1);
    addr = map_anon_region(4096 * 10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * 4096));
        ASSERT(get_swap_offset_from_page(addr + (i * 4096)) == i % 3);
        ASSERT(addr[i] == i);
    }
    ASSERT(get_swapfile_count() == 0);
}

int main(void) {
    return run_all_tests();
}
