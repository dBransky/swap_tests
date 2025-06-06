// test_util.c
#include "test_util.h"
#include <string.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <sys/swap.h>
#include <sys/mman.h>
#include <signal.h>
#include <limits.h>
#include <time.h>


struct swap_info_args {
    void *virtual_address;     // Input: User-space virtual address
    unsigned long offset;      // Output: Swap offset
    int has_swap_info;         // Output: Swap info presence
};

#define PAGE_SIZE 4096
#define TOTAL_SWAPFILES 200
static int free_swapfile_index = 1;
int is_measuring = 0;
struct timespec start_time;

void start_measurement(void) {
    if (is_measuring) {
        fprintf(stderr, "Measurement already started.\n");
        exit(EXIT_FAILURE);
    }
    is_measuring = 1;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    printf("Starting measurement... at %llu ns\n", now.tv_sec * 1000000000ULL + now.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}
unsigned long long stop_measurement(void) {
    if (!is_measuring) {
        fprintf(stderr, "Measurement not started.\n");
        exit(EXIT_FAILURE);
    }
    is_measuring = 0;
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    unsigned long long start_ns = start_time.tv_sec * 1000000000ULL + start_time.tv_nsec;
    unsigned long long end_ns = end_time.tv_sec * 1000000000ULL + end_time.tv_nsec;
    printf("Stopping measurement... at %llu ns\n", end_ns);
    return (end_ns - start_ns) / 1000; // Convert to microseconds
}

void set_minimal_swapfile_num(int num){
    if (num < 1 || num > TOTAL_SWAPFILES) {
        fprintf(stderr, "Invalid number of swapfiles: %d. Must be between 1 and %d.\n", num, TOTAL_SWAPFILES);
        exit(EXIT_FAILURE);
    }
    free_swapfile_index = num;
}

pid_t start_ftrace(void) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // close(STDOUT_FILENO); // Close stdout in the child process
        close(STDERR_FILENO); // Close stderr in the child process
        // Child process - exec trace-cmd
        char *args[] = {
            "trace-cmd",
            "record",
            "-e",
            "swap:*",
            "-e",
            "vmscan:*",
            NULL
            // "-e",
            // "kmem:*",
            // "-e",
            // "mmap:*",
            // "-e",
            // "vmalloc:*",
        };
        
        execvp("trace-cmd", args);
        perror("execvp failed");
        exit(1);
    }
    sleep(10); // Give trace-cmd some time to start
    return pid;
}
void stop_ftrace(char* test_name, pid_t pid) {
    kill(pid, SIGINT); // Send SIGINT to stop trace-cmd
    sleep(10); // Wait for trace-cmd to finish
    kill(pid, SIGKILL); // Ensure it is killed
    char* command = malloc(256);
    snprintf(command, 256, "trace-cmd report > %s.trace", test_name);
    system(command);
    free(command);
}
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
    // printf("%p\n",args.virtual_address);
    if (ioctl(open(DEVICE, O_RDONLY), IOCTL_GET_SWAP_OFFSET_FROM_PAGE, &args) < 0) {
        perror("Failed to get swap offset from page");
        return -1;
    }
    return args.offset;
}
void mkswap(const char *filename){
    char command[256];
    snprintf(command, sizeof(command), "mkswap %s > /dev/null 2>&1", filename);
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

int swapout_page(void *addr) {
    void* aligned_page = (void*)((unsigned long)addr - (unsigned long)addr % PAGE_SIZE);
    printf("Swapping out page at address %p aligned page %p\n", addr, aligned_page);
    if (madvise(aligned_page, PAGE_SIZE, MADV_PAGEOUT) < 0) {
        perror("madvise");
        return -1;
    }
    return 0;
}
void* map_large_anon_region(unsigned long long size) {
    if (size % (unsigned long long)PAGE_SIZE != 0) {
        fprintf(stderr, "Size must be a multiple of PAGE_SIZE (%d)\n", PAGE_SIZE);
        return NULL;
    }
    unsigned long long sz_in_pages = (size / (unsigned long long)PAGE_SIZE);
    char *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    for (unsigned long long i = 0; i < sz_in_pages; i++) {
        *(unsigned long long *)(addr+(i * PAGE_SIZE)) = i;
    }
    return addr;
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
        addr[i*PAGE_SIZE] = i;
    }
    return addr;
}
struct vma_info_args get_vma_info(void *addr) {
    struct vma_info_args args = {0};
    args.virtual_address = addr;
    if (ioctl(open(DEVICE, O_RDONLY), IOCTL_VMA_INFO, &args) < 0) {
        perror("Failed to get VMA info");
        return args; // Return empty args on error
    }
    return args;
}

int disable_swaps() {
    FILE *fp;
    char line[256];
    char filename[256];
    char type[32];
    int size, used, priority;
    int min_available_index = 1;
    
    // Open /proc/swaps
    fp = fopen("/proc/swaps", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/swaps");
        return -1;
    }
    
    // Skip the header line
    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error reading /proc/swaps header\n");
        fclose(fp);
        return -1;
    }
    
    // Process each swap entry
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Parse the line
        if (sscanf(line, "%255s %31s %d %d %d", filename, type, &size, &used, &priority) != 5) {
            continue; // Skip malformed lines
        }
        printf("line: %s\n", line);
        printf("Processing: %s (used: %d)\n", filename, used);
        char *underscore = strrchr(filename, '_');
        char *dot = strrchr(filename, '.');
        int index = INT_MAX;
        if (underscore && dot && underscore < dot) {
            index = atoi(underscore + 1);
        }
        
        // If used is 0, turn off the swapfile
        if (used == 0) {
            printf("Turning off unused swapfile: %s\n", filename);
            disable_swap(filename);
            // Extract index from filename
            // Look for pattern like "swapfile_X.swap"
            
        }
        else{
            printf("Swapfile %s is in use, index: %d. min_available_index: %d\n", filename, index, min_available_index);

            if (index+1 > min_available_index) {
                min_available_index = index + 1; // Increment to find the next available index
            }
        }
    }
    
    fclose(fp);
    printf("Minimal available swapfile index: %d\n", min_available_index);
    return min_available_index;
}
