// test_util.c
#include "test_util.h"
#include <string.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <sys/swap.h>
#define PAGE_SIZE 4096
#define TOTAL_SWAPFILES 100
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
    unsigned long offset;
    if (ioctl(open(DEVICE, O_RDONLY), IOCTL_GET_SWAP_OFFSET_FROM_PAGE, &addr) < 0) {
        perror("Failed to get swap offset from page");
        return -1;
    }
    return offset;
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
    for (int i = 1; i <= num_swapfiles; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "/scratch/vma_swaps/swapfile_%d.swap", i);
        mkswap(filename);
        enable_swap(filename,swap_flags);
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
int swapout_page(void *addr) {
    if (madvise(addr, MADV_PAGEOUT, 0) < 0) {
        perror("madvise");
        return -1;
    }
    return 0;
}
void* map_anon_region(size_t size) {
    char *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    for (char i = 0; i < size / PAGE_SIZE; i++) {
        addr[i] = i;
    }
    return addr;
}