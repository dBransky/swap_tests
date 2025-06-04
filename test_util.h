// test_util.h
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/swapctl"
#define IOCTL_GET_SWAPFILE_COUNT _IOR('s', 0x01, int)
#define IOCTL_GET_SWAP_OFFSET_FROM_PAGE _IOR('s', 0x02, unsigned long)
#define IOCTL_VMA_HAS_SWAP_INFO _IOR('s', 0x03, int)


int swapout_page(void *addr);
int get_swapfile_count();
int get_swap_offset_from_page(void *addr);
void make_swaps(int num_swapfiles, int swap_flags);
int vma_has_swap_info(void *addr);
int delete_all_swaps();
int delete_all_swaps_no_check();
void* map_anon_region(size_t size);

#endif // TEST_UTIL_H
