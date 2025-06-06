#include "test_framework.h"
#include "test_util.h" // for make_swaps()
#include <sys/mman.h>
#include <getopt.h>

#define PAGE_SIZE 4096
REGISTER_TEST(test_folio_offset);
REGISTER_TEST(test_multiple_swapfiles);
REGISTER_TEST(test_multiple_swapfiles2);
REGISTER_TEST(test_vma_si_allcation);
REGISTER_TEST(test_stack_vma_offset);
REGISTER_TEST(test_stack_vma_enlarge);
REGISTER_TEST(test_available_swapfile);
REGISTER_TEST(test_vma_values);
REGISTER_TEST(test_mul_vma_values);

void test_stack_vma_offset(void) {
    make_swaps(1, 0);
    char* stack = mmap(NULL, PAGE_SIZE * 10, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    // stack += PAGE_SIZE;
    ASSERT(stack != MAP_FAILED);
    for (int i = 0; i < 10; i++) {
        stack[i * PAGE_SIZE] = i;
    }
    for (int i = 0; i < 10; i++) {
        swapout_page(stack + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(stack + (i * PAGE_SIZE)), 9-i);
        stack[i * PAGE_SIZE] = i;
    }
}
void test_stack_vma_enlarge(void) {
    char dummy[PAGE_SIZE * 10];
    struct vma_info_args vma_info = get_vma_info(dummy);
    int base_offset = DIV_ROUND_UP(vma_info.vma_end - (unsigned long)dummy, PAGE_SIZE) - 1;
    printf("Base offset: %d\n", base_offset);

    void test_func(){
        char dummy [PAGE_SIZE];
        dummy[0] = 10;
        swapout_page(dummy);
        ASSERT_EQ(get_swap_offset_from_page(dummy), base_offset+1);
        ASSERT_EQ(dummy[0], 10);

    }
    // get the base offset from the vma using get_vma_info

    
    make_swaps(1, 0);
    for (int i = 0; i < 10; i++) {
        dummy[i * PAGE_SIZE] = i;
        swapout_page(dummy + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(dummy + (i * PAGE_SIZE)), (base_offset) -i);
        ASSERT_EQ(dummy[i * PAGE_SIZE], i);
    }
    test_func();
}

void test_vma_si_allcation(void) {
    make_swaps(1, 0);
    char *addr = map_anon_region(PAGE_SIZE * 10);
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(vma_has_swap_info(addr + (i * PAGE_SIZE)), 0);
    }
    swapout_page(addr);
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(vma_has_swap_info(addr + (i * PAGE_SIZE)), 1);
    }
}
void test_available_swapfile(void) {
    ASSERT(get_swapfile_count() == 0);
    make_swaps(1, 0);
    ASSERT(get_swapfile_count() == 1);
    // mmap an anon region
    void *addr = map_anon_region(PAGE_SIZE);
    ASSERT(addr != NULL);
    swapout_page(addr);
    ASSERT(get_swapfile_count() == 0);
}
void test_folio_offset(void) {
    make_swaps(1, 0);
    char *addr = map_anon_region(PAGE_SIZE*10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr[i * PAGE_SIZE], i);
    }
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * PAGE_SIZE));
    }
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(get_swap_offset_from_page(addr + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr[i * PAGE_SIZE], i);
    }
}
void test_multiple_swapfiles(void) {
    make_swaps(3, 0);
    ASSERT_EQ(get_swapfile_count(), 3);
    char *addr = map_anon_region(PAGE_SIZE * 10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr[i*PAGE_SIZE], i);
    }
    ASSERT_EQ(get_swapfile_count(), 2);
    char *addr2 = map_anon_region(PAGE_SIZE * 10);
    ASSERT(addr2 != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr2 + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr2 + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr2[i*PAGE_SIZE], i);
    }
    ASSERT_EQ(get_swapfile_count(), 1);
    char* addr3 = map_anon_region(PAGE_SIZE * 10);
    ASSERT(addr3 != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr3 + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr3 + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr3[i*PAGE_SIZE], i);
    }
    ASSERT_EQ(get_swapfile_count(), 0);
}

void test_multiple_swapfiles2(void) {
    make_swaps(2, 0);
    ASSERT_EQ(get_swapfile_count(), 2);
    char *addr = map_anon_region(PAGE_SIZE * 10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr[i*PAGE_SIZE], i);
    }
    ASSERT_EQ(get_swapfile_count(), 1);
    char *addr2 = map_anon_region(PAGE_SIZE * 10);
    ASSERT(addr2 != NULL);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr2 + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr2 + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr2[i*PAGE_SIZE], i);
    }
    ASSERT_EQ(get_swapfile_count(), 0);
    // go back to the first address and swapout again
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr + (i * PAGE_SIZE)), i);
        ASSERT_EQ(addr[i*PAGE_SIZE], i);
    }

}
void test_vma_values(void){
    make_swaps(1, 0);
    char* addr = map_anon_region(PAGE_SIZE * 10);
    struct vma_info_args vma_info = get_vma_info(addr);
    ASSERT_EQ(vma_info.vma_start, (unsigned long)addr);
    ASSERT_EQ(vma_info.virtual_address, addr);
    ASSERT_EQ(vma_info.swap_info, NULL);
    swapout_page(addr);
    struct vma_info_args vma_info2 = get_vma_info(addr);
    ASSERT_EQ(vma_info2.vma_start, (unsigned long)addr);
    ASSERT_EQ(vma_info2.virtual_address, addr);
    ASSERT_NEQ(vma_info2.swap_info, NULL);
    swapout_page(addr + PAGE_SIZE * 5);
    struct vma_info_args vma_info3 = get_vma_info(addr + PAGE_SIZE * 5);
    ASSERT_EQ(vma_info3.vma_start, (unsigned long)addr);
    ASSERT_EQ(vma_info3.virtual_address, addr + PAGE_SIZE * 5);
    ASSERT_NEQ(vma_info3.swap_info, NULL);
    ASSERT_EQ(vma_info3.swap_info, vma_info2.swap_info);
}

void test_mul_vma_values(void){
    make_swaps(2, 0);
    char* addr = map_anon_region(PAGE_SIZE * 10);
    struct vma_info_args vma_info = get_vma_info(addr);
    ASSERT_EQ(vma_info.vma_start, (unsigned long)addr);
    ASSERT_EQ(vma_info.virtual_address, addr);
    ASSERT_EQ(vma_info.swap_info, NULL);
    swapout_page(addr);
    struct vma_info_args vma_info2 = get_vma_info(addr);
    ASSERT_NEQ(vma_info2.swap_info, NULL);

    char* addr2 = map_anon_region(PAGE_SIZE * 10);
    struct vma_info_args vma_info3 = get_vma_info(addr2);
    ASSERT_EQ(vma_info3.virtual_address, addr2);
    ASSERT_EQ(vma_info3.swap_info, NULL);
    // read the first again
    struct vma_info_args vma_info4 = get_vma_info(addr);
    //check if merged
    if(vma_info3.vma_start == vma_info4.vma_start){
        printf("Merged VMA: %lx - %lx\n", vma_info3.vma_start, vma_info3.vma_end);
        ASSERT_EQ(vma_info4.vma_end, vma_info3.vma_end);
        ASSERT_EQ(vma_info4.swap_info, NULL);
    } else {
        ASSERT_NEQ(vma_info4.vma_end, vma_info3.vma_end);
        ASSERT_NEQ(vma_info4.swap_info, NULL);
    }
    swapout_page(addr2);
    //read both again
    struct vma_info_args vma_info5 = get_vma_info(addr);
    struct vma_info_args vma_info6 = get_vma_info(addr2);
    if(vma_info5.vma_start == vma_info6.vma_start) {
        printf("Merged VMA: %lx - %lx\n", vma_info5.vma_start, vma_info5.vma_end);
        ASSERT_EQ(vma_info5.vma_end, vma_info6.vma_end);
        ASSERT_NEQ(vma_info5.swap_info, NULL);
        ASSERT_NEQ(vma_info6.swap_info, NULL);
        ASSERT_EQ(vma_info5.swap_info, vma_info6.swap_info);
    } else {
        ASSERT_NEQ(vma_info5.vma_end, vma_info6.vma_end);
        ASSERT_NEQ(vma_info5.swap_info, NULL);
        ASSERT_NEQ(vma_info6.swap_info, NULL);
        ASSERT_NEQ(vma_info5.swap_info, vma_info6.swap_info);
    }
}

int main(int argc, char *argv[]) {
    // add cli with getopt
    static int minimal_swapfile_num = 1;
    static int enable_traces = 0;
    static struct option long_options[] = {
        {"trace", no_argument, &enable_traces, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int opt;
    int option_index = 0;
    void print_usage() {
        printf("Usage: %s [--trace] [--minimal-swapfile-num <num>]\n", argv[0]);
        printf("Options:\n");
        printf("  --trace                   Enable tracing with trace-cmd\n");
        printf("  -h, --help                Show this help message\n");
    }
    while ((opt = getopt_long(argc, argv, "th", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                enable_traces = 1;
                break;
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            default:
                print_usage();
                exit(EXIT_FAILURE);
            }
        }
        
    set_minimal_swapfile_num(minimal_swapfile_num);
    return run_all_tests(enable_traces);
}
