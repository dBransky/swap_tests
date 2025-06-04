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
int swapout_page(void *addr);
int get_swapfile_count();
int get_swap_offset_from_page(void *addr);
void make_swaps(int num_swapfiles, int swap_flags);
int vma_has_swap_info(void *addr);
int delete_all_swaps();
void* map_anon_region(size_t size);

#endif // TEST_UTIL_H
