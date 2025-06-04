// test_util.c
#include "test_util.h"
#include <string.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <sys/swap.h>
#include <sys/mman.h>

struct swap_info_args {
    void *virtual_address;     // Input: User-space virtual address
    unsigned long offset;      // Output: Swap offset
    int has_swap_info;         // Output: Swap info presence
};

#define PAGE_SIZE 4096
#define TOTAL_SWAPFILES 100
static int free_swapfile_index = 60;

int vma_has_swap_info(void *addr) {
    struct swap_info_args args = {0};
    args.has_swap_info = -1;
    args.virtual_address = addr;
    if (ioctl(open(DEVICE, O_RDONLY), IOCTL_VMA_HAS_SWAP_INFO, &args) < 0) {
        perror("Failed to check VMA swap info");
        return -1;
    }
    return args.has_swap_info;
}
int get_swapfile_count( ){
    int count;
    int fd = open(DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    if (ioctl(fd, IOCTL_GET_SWAPFILE_COUNT, &count) < 0) {
        perror("Failed to get swapfile count");
        return -1;
    }
    return count;
}
int get_swap_offset_from_page(void *addr) {
    struct swap_info_args args = {0};
    args.virtual_address = addr;
    if (ioctl(open(DEVICE, O_RDONLY), IOCTL_GET_SWAP_OFFSET_FROM_PAGE, &args) < 0) {
        perror("Failed to get swap offset from page");
        return -1;
    }
    return args.offset;
}
void mkswap(const char *filename){
    char command[256];
    snprintf(command, sizeof(command), "mkswap %s", filename);
    system(command);
}
void enable_swap(const char *filename, int swap_flags) {
    // printf("Enabling swap on %s\n", filename);
    int ret = syscall(SYS_swapon, filename, swap_flags);
    if (ret < 0) {
        perror("swapon");
    }
}
void disable_swap(const char *filename) {
    // printf("Disabling swap on %s\n", filename);
    if (swapoff(filename) < 0) {
        perror("Failed to disable swap");
        exit(EXIT_FAILURE);
    }
}
void make_swaps(int num_swapfiles, int swap_flags) {
    for (int i = 0; i < num_swapfiles; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "/scratch/vma_swaps/swapfile_%d.swap", i+free_swapfile_index);
        mkswap(filename);
        enable_swap(filename,swap_flags);
        free_swapfile_index++;
        if (free_swapfile_index > TOTAL_SWAPFILES) {
            fprintf(stderr, "Reached maximum number of swapfiles (%d)\n", TOTAL_SWAPFILES);
            exit(EXIT_FAILURE);
        }
    }
}

int delete_all_swaps() {
    for (int i = 1; i <= TOTAL_SWAPFILES; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "/scratch/vma_swaps/swapfile_%d.swap", i);
        disable_swap(filename);
        if (remove(filename) < 0) {
            perror("Failed to delete swap file");
            return -1;
        }
    }
    return 0;
}

int delete_all_swaps_no_check() {
    for (int i = 1; i <= TOTAL_SWAPFILES; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "/scratch/vma_swaps/swapfile_%d.swap", i);
        swapoff(filename);
    }
    return 0;
}
int swapout_page(void *addr) {
    if (madvise(addr, 1, MADV_PAGEOUT) < 0) {
        perror("madvise");
        return -1;
    }
    return 0;
}
void* map_anon_region(size_t size) {
    if (size % PAGE_SIZE != 0) {
        fprintf(stderr, "Size must be a multiple of PAGE_SIZE (%d)\n", PAGE_SIZE);
        return NULL;
    }
    char sz_in_pages = (size / PAGE_SIZE);
    char *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    for (int i = 0; i < sz_in_pages; i++) {
        addr[i] = i;
    }
    return addr;
}