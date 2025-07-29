#include "test_framework.h"
#include "test_util.h" // for make_swaps()
#include <sys/mman.h>
#include <getopt.h>

#define PAGE_SIZE 4096
// REGISTER_TEST(test_folio_offset);
// REGISTER_TEST(test_multiple_swapfiles);
// REGISTER_TEST(test_multiple_swapfiles2);
// REGISTER_TEST(test_vma_si_allcation);
// REGISTER_TEST(test_stack_vma_offset);
// REGISTER_TEST(test_stack_vma_enlarge);
// REGISTER_TEST(test_available_swapfile);
// REGISTER_TEST(test_vma_values);
// REGISTER_TEST(test_mul_vma_values);
// REGISTER_TEST(test_heap_enlarge);
// REGISTER_TEST(test_eviction);
// REGISTER_PERF_TEST(test_seq_swapout_throughput);
// REGISTER_PERF_TEST(test_rand_swapout_throughput);
REGISTER_PERF_TEST(test_seq_swapin_throughput);
// REGISTER_PERF_TEST(test_rand_swapin_throughput);
/**TODO: 
    -add shared vma tests
    -add heap recude tests
    -add stack reduce tests
    -add vma merge tests. if merge to the right do not NULL the swap_info
**/

void test_eviction(void) {
    make_swaps(1, 0);
    unsigned long long region_size = 2<<28; // 256MiB region
    unsigned long long pages = region_size / PAGE_SIZE;
    char *addr = map_large_anon_region(region_size);
    ASSERT(addr != NULL);
    for (unsigned long i = 0; i < pages; i++) {
        addr[i * PAGE_SIZE] = i;
    }
    evict_mem(100000);
    sleep(5);
}

void test_heap_enlarge(void) {
    make_swaps(1, 0);
    char* addr = malloc(PAGE_SIZE * 10);
    ASSERT(addr != NULL);
    for (int i = 0; i < 10; i++) {
        addr[i * PAGE_SIZE] = i;
    }
    swapout_page(addr);
    int base_offset = get_swap_offset_from_page(addr);
    printf("Base offset: %d\n", base_offset);
    for (int i = 0; i < 10; i++) {
        swapout_page(addr + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr + (i * PAGE_SIZE)), base_offset + i);
        ASSERT_EQ(addr[i * PAGE_SIZE], i);
    }
    char* addr2 = malloc(PAGE_SIZE * 10);
    ASSERT(addr2 != NULL);
    for (int i = 0; i < 10; i++) {
        addr2[i * PAGE_SIZE] = i;
    }
    for (int i = 0; i < 10; i++) {
        swapout_page(addr2 + (i * PAGE_SIZE));
        ASSERT_EQ(get_swap_offset_from_page(addr2 + (i * PAGE_SIZE)), base_offset + 10 + i);
        ASSERT_EQ(addr2[i * PAGE_SIZE], i);
    }

}
#define SWAP_FLAG_SYNCHRONOUS_IO_W	0x100000
void test_seq_swapin_throughput(void) {
    make_swaps(1, SWAP_FLAG_SYNCHRONOUS_IO_W);
    unsigned long long region_size = 1<<29; // 512MiB region
    unsigned long long pages = region_size / PAGE_SIZE;
    char *addr = map_large_anon_region(region_size);
    for (unsigned long long i = 0; i < pages; i++) {
        swapout_page(addr + (i * PAGE_SIZE));
    }
    sleep(5); // wait for eviction to complete
    evict_mem(region_size/ PAGE_SIZE);
    sleep(5); // wait for eviction to complete
    start_measurement();
    for (unsigned long long i = 0; i < pages; i++) {
        unsigned long long * tmp_addr = (unsigned long long *)(addr+(i * PAGE_SIZE));
        (*tmp_addr)++;
        ASSERT_EQ(*tmp_addr, i+1);

    }
    double elapsed = stop_measurement();
    printf("took %.2f seconds to swap in %llu pages\n", elapsed, pages);
    double throughput = (double)(region_size)/(1<<20) / elapsed; // MB per second
    printf("Sequential swapin throughput: %.2f MB/s\n", throughput);
    ASSERT_ABOVE(throughput, 150);
}


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
    static int will_run_perf_tests = 0;
    static struct option long_options[] = {
        {"trace", no_argument, &enable_traces, 't'},
        {"perf", no_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int opt;
    int option_index = 0;
    void print_usage() {
        printf("Usage: %s [--trace] [--minimal-swapfile-num <num>]\n", argv[0]);
        printf("Options:\n");
        printf("  --trace                   Enable tracing with trace-cmd\n");
        printf("  --perf                   run preformace tests\n");
        printf("  -h, --help                Show this help message\n");
    }
    while ((opt = getopt_long(argc, argv, "thp", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                will_run_perf_tests = 1;
                break;
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
    run_all_tests(enable_traces);
    if (will_run_perf_tests) {
        run_perf_tests(enable_traces);
    }
    return 0;
}
