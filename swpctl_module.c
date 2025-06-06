#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/swap.h>
#include <linux/mutex.h>
#include <linux/plist.h>
#include <linux/swapops.h>

#define DEVICE_NAME "swapctl"
#define IOCTL_GET_SWAPFILE_COUNT _IOR('s', 0x01, int)
#define IOCTL_GET_SWAP_OFFSET_FROM_PAGE _IOR('s', 0x02, unsigned long)
#define IOCTL_VMA_HAS_SWAP_INFO _IOR('s', 0x03, int)
#define IOCTL_VMA_INFO _IOR('s', 0x04, struct vma_info_args)

struct swap_info_args {
    void *virtual_address;     // Input: User-space virtual address
    unsigned long offset;      // Output: Swap offset
    int has_swap_info;         // Output: Swap info presence
};

struct vma_info_args {
    void *virtual_address;
    unsigned long vma_start;
    unsigned long vma_end;
    void *vma_ptr;
    unsigned long vm_flags;
    void *swap_info;
};

static long swapctl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    
    switch (cmd) {
    case IOCTL_VMA_INFO: {
        struct vma_info_args args;

        if (copy_from_user(&args, (void __user *)arg, sizeof(args)))
            return -EFAULT;

        struct vm_area_struct *vma = find_vma(current->mm, (unsigned long)args.virtual_address);
        if (!vma)
            return -EINVAL;

        args.vma_start = vma->vm_start;
        args.vma_end = vma->vm_end;
        args.vma_ptr = vma;
        args.vm_flags = vma->vm_flags;
        spin_lock(&vma->swap_lock);
        args.swap_info = vma->si; // Pointer to swap info
        spin_unlock(&vma->swap_lock);

        if (copy_to_user((void __user *)arg, &args, sizeof(args)))
            return -EFAULT;

        return 0;
    }
    case IOCTL_GET_SWAPFILE_COUNT: {
        int count = get_avail_swap_info_count();
        if (copy_to_user((int __user *)arg, &count, sizeof(count)))
            return -EFAULT;
        return 0;
        }
    case IOCTL_GET_SWAP_OFFSET_FROM_PAGE: {
        struct swap_info_args args;

        if (copy_from_user(&args, (void __user *)arg, sizeof(args)))
            return -EFAULT;
        printk(KERN_INFO "swapctl: Getting swap offset for address 0x%lx\n", (unsigned long)args.virtual_address);
         // Pin the user page

        struct page *page = NULL;
        int ret = get_user_pages_fast((unsigned long)args.virtual_address, 1, 0, &page);
        if (ret != 1) {
            pr_err("swapctl: Failed to get page for user address %px (ret=%d)\n", 
                args.virtual_address, ret);
            return -EFAULT;
        }
        struct folio* folio = page_folio(page);
        if(!folio) {
            pr_err("swapctl: Invalid folio for address %px\n", args.virtual_address);
            return -EINVAL;
        }
        printk(KERN_INFO "swapctl: Folio %px has swap %lu\n", folio, swp_offset(folio->swap));
        args.offset = swp_offset(folio->swap);
        
        put_page(page); // ADD THIS LINE before return

        if (copy_to_user((void __user *)arg, &args, sizeof(args)))
            return -EFAULT;

        return 0;
    }
    case IOCTL_VMA_HAS_SWAP_INFO: {
        struct swap_info_args args;

        if (copy_from_user(&args, (void __user *)arg, sizeof(args)))
            return -EFAULT;

        struct vm_area_struct *vma = find_vma(current->mm, (unsigned long)args.virtual_address);
        if (!vma)
            return -EINVAL;

        spin_lock(&vma->swap_lock);
        args.has_swap_info = vma->si != NULL;
        spin_unlock(&vma->swap_lock);

        if (copy_to_user((void __user *)arg, &args, sizeof(args)))
            return -EFAULT;

        return 0;
    }
    default:
        return -EINVAL;
}
}
static const struct file_operations swapctl_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = swapctl_ioctl,
    .compat_ioctl = swapctl_ioctl,
};

static struct miscdevice swapctl_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &swapctl_fops,
    .mode = 0666,
};

static int __init swapctl_init(void)
{
    int ret = misc_register(&swapctl_dev);
    if (ret)
        pr_err("swapctl: failed to register misc device\n");
    else
        pr_info("swapctl: device registered as /dev/%s\n", DEVICE_NAME);
    return ret;
}

static void __exit swapctl_exit(void)
{
    misc_deregister(&swapctl_dev);
    pr_info("swapctl: module unloaded\n");
}

module_init(swapctl_init);
module_exit(swapctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel");
MODULE_DESCRIPTION("Expose swapfile stats");
