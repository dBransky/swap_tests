#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/swap.h>
#include <linux/mutex.h>
#include <linux/plist.h>

#define DEVICE_NAME "swapctl"
#define IOCTL_GET_SWAPFILE_COUNT _IOR('s', 0x01, int)
#define IOCTL_GET_SWAP_OFFSET_FROM_PAGE _IOR('s', 0x02, unsigned long)
#define IOCTL_VMA_HAS_SWAP_INFO _IOR('s', 0x03, int)

extern struct plist_head *swap_avail_heads;
extern spinlock_t swap_avail_lock;

static long swapctl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    
    switch (cmd) {
        case IOCTL_GET_SWAPFILE_COUNT: {
        int count = get_avail_swap_info_count();
        if (copy_to_user((int __user *)arg, &count, sizeof(count)))
            return -EFAULT;
        return 0;
        }
    case IOCTL_GET_SWAP_OFFSET_FROM_PAGE: {
        void* virtual_address;
        unsigned long offset;
        if (copy_from_user(&virtual_address, (void __user *)arg, sizeof(virtual_address)))
            return -EFAULT;
        struct folio *folio = virt_to_folio(virtual_address);
        offset = swp_offset(folio->swap);
        if (copy_to_user((unsigned long __user *)arg, &offset, sizeof(offset)))
            return -EFAULT;
        return 0;
    }
    case IOCTL_VMA_HAS_SWAP_INFO: {
        void* virtual_address;
        int has_swap_info;
        if (copy_from_user(&virtual_address, (void __user *)arg, sizeof(virtual_address)))
            return -EFAULT;
        struct vm_area_struct *vma = find_vma(current->mm, (unsigned long)virtual_address);
        if (!vma)
            return -EINVAL;
        spin_lock(&vma->swap_lock);
    	has_swap_info  = vma->si != NULL;
    	spin_unlock(&vma->swap_lock);

        if (copy_to_user((int __user *)arg, &has_swap_info, sizeof(has_swap_info)))
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
