#include <linux/file.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>

#include "nimbos.h"
#include "syscall_handler.h"

static const struct vm_operations_struct
shadow_physical_vm_ops = {
#ifdef CONFIG_HAVE_IOREMAP_PROT
    .access = generic_access_phys,
#endif
};

static int nimbos_register_syscall_handler(void)
{
    return add_syscall_handler(get_current());
}

static int nimbos_deregister_syscall_handler(syscall_handler_t handler)
{
    return del_syscall_handler(handler);
}

static int nimbos_syscall_setup(void)
{
    return nimbos_register_syscall_handler();
}

int nimbos_open(struct inode *inode, struct file *file)
{
    pr_info("nimbos_open: %p(%d)\n", get_current(), get_current()->pid);
    return 0;
}

int nimbos_close(struct inode *inode, struct file *file)
{
    pr_info("nimbos_close: %p(%d)\n", get_current(), get_current()->pid);
    nimbos_deregister_syscall_handler(get_current());
    return 0;
}

long nimbos_ioctl(struct file *file, unsigned int ioctl, unsigned long arg)
{
    long err;

    switch (ioctl) {
    case NIMBOS_SYSCALL_SETUP:
        err = nimbos_syscall_setup();
        break;
    default:
        err = -EINVAL;
        break;
    }
    return err;
}

int nimbos_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long paddr;
    unsigned long size;
    unsigned long map_size;

    map_size = vma->vm_end - vma->vm_start;
    paddr = rt_region.start + (vma->vm_pgoff << PAGE_SHIFT);
    size = rt_region.size - (vma->vm_pgoff << PAGE_SHIFT);

    if (vma->vm_end < vma->vm_start){
        return -EINVAL;
    }

    if (paddr <= rt_region.start || paddr >= rt_region.start + rt_region.size) {
        return -EINVAL;
    }

    pr_info("nimbos_mmap: [0x%lx, 0x%lx) pgoff=%lx => paddr=0x%lx, size=0x%lx\n",
             vma->vm_start, vma->vm_end, vma->vm_pgoff, paddr, map_size);

    if (map_size > size) {
        return -ENOMEM;
    }

    vma->vm_ops = &shadow_physical_vm_ops;
    return remap_pfn_range(vma, vma->vm_start, paddr >> PAGE_SHIFT, map_size,
                           vma->vm_page_prot);
}
