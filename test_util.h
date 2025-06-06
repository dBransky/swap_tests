// test_util.h
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct vma_info_args {
    void *virtual_address;
    unsigned long vma_start;
    unsigned long vma_end;
    void *vma_ptr;
    unsigned long vm_flags;
    void *swap_info;
};

#define DEVICE "/dev/swapctl"
#define IOCTL_GET_SWAPFILE_COUNT _IOR('s', 0x01, int)
#define IOCTL_GET_SWAP_OFFSET_FROM_PAGE _IOR('s', 0x02, unsigned long)
#define IOCTL_VMA_HAS_SWAP_INFO _IOR('s', 0x03, int)
#define IOCTL_VMA_INFO _IOR('s', 0x04, struct vma_info_args)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))


int swapout_page(void *addr);
int get_swapfile_count();
int get_swap_offset_from_page(void *addr);
void make_swaps(int num_swapfiles, int swap_flags);
int vma_has_swap_info(void *addr);
int disable_swaps();
void* map_anon_region(size_t size);
pid_t start_ftrace(void);
void stop_ftrace(char* test_name, pid_t pid);
struct vma_info_args get_vma_info(void *addr);
void set_minimal_swapfile_num(int num);

#endif // TEST_UTIL_H
