// SPDX-License-Identifier: GPL-2.0
// This device uses alloc_pages to request contiguous physical mem allocation.
// Allocation results in page-aligned chunks of physical memory.
// 1. ioremap alternative can be used but is less flexible since requires to explicitly
// reserve memory at boot (memmap=16K$0x10000000=MEM_SIZE|PHYS_ADDR). Usually used
// to map I/O HW mem directly.
// 2. kmalloc alternative uses kernel page allocator to map contiguous low phys mem.
// 3. vmalloc allocates kernel virtual memory and is not contiguous (physical), only virtually.
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/mmzone.h>
#include <linux/gfp.h>
#include <asm-generic/memory_model.h>

static const size_t KERNEL_PAGE_SIZE = (1UL << CONFIG_PAGE_SHIFT);
static const size_t MEM_SIZE = KERNEL_PAGE_SIZE * 2;
static const char DEVICE_NAME[] = "mmap_phys_dev";
static unsigned int device_major;

static unsigned long reserved_mem_phys;
static struct page *reserved_page;

static int get_mem_order(unsigned long size)
{
	return ilog2((size - 1) >> CONFIG_PAGE_SHIFT) + 1;
}

static int reserve_phys_mem(void)
{
	// Reserve contiguous pages
	reserved_page = alloc_pages(GFP_KERNEL, get_mem_order(MEM_SIZE));

	if (!reserved_page) {
		pr_err("%s: failed to alloc %luB contiguous mem\n", DEVICE_NAME,
		       MEM_SIZE);
		return -ENOMEM;
	}

	// Convert the page to a physical address
	reserved_mem_phys = page_to_pfn(reserved_page);

	pr_info("%s: physical mem(%luB) reserved in kernel space\n",
	        DEVICE_NAME, MEM_SIZE);
	return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
	pr_info("%s: open\n", DEVICE_NAME);
	return 0;
}

static int device_close(struct inode *inode, struct file *file)
{
	pr_info("%s: close\n", DEVICE_NAME);
	return 0;
}

// maps vma.size of mem requested by user, from kernel.phys.start_addr + vma.offset:
// user.start_addr = kernel.phys.addr + vma.offset
// user.end_addr = user.start_addr + vma.size
static int device_mmap(struct file *file, struct vm_area_struct *vma)
{
	// Convert physical address to page frame number
	unsigned long pfn = reserved_mem_phys >> CONFIG_PAGE_SHIFT;
	unsigned long user_size = vma->vm_end - vma->vm_start;
	// convert page offset in bytes
	unsigned long requested_offset_bytes =
		vma->vm_pgoff << CONFIG_PAGE_SHIFT;
	const unsigned long user_reserved_size =
		MEM_SIZE - requested_offset_bytes;

	if (user_size > user_reserved_size) {
		pr_err("%s: requested mmap size > %lu\n", DEVICE_NAME,
		       user_reserved_size);
		return -EINVAL;
	}

	if (remap_pfn_range(vma, vma->vm_start, (pfn + vma->vm_pgoff),
	                    user_size,
	                    vma->vm_page_prot)) {
		pr_err(
			"%s: failed to remap physical mem to userspace:",
			DEVICE_NAME);
		pr_err(
			"%s: user.start(0x%lx), kernel.phys.start.page(0x%lx), user.size(0x%lx)",
			DEVICE_NAME, vma->vm_start,
			(pfn + vma->vm_pgoff),
			user_size);
		return -EIO;
	}

	pr_info(
		"%s: Reserved mem mapped to userspace (low-mem=kernel, high-mem=user):",
		DEVICE_NAME);
	pr_info(
		"%s: kernel: phys.start(0x%lx), phys.end(0x%lx), phys.size(0x%lx)",
		DEVICE_NAME, reserved_mem_phys, reserved_mem_phys+MEM_SIZE,
		MEM_SIZE);
	pr_info(
		"%s: user: vmem.start(0x%lx), vmem.end(0x%lx), vmem.size(0x%lx)",
		DEVICE_NAME,
		vma->vm_start, vma->vm_start+user_size, user_size);

	return 0;
}

static struct file_operations fops = {
	.open = device_open,
	.release = device_close,
	.mmap = device_mmap,
};

static int __init mymodule_init(void)
{
	if (reserve_phys_mem()) {
		pr_err("%s: failed to reserve memory\n", DEVICE_NAME);
		return -ENOMEM;
	}
	// Register character device
	device_major = register_chrdev(0, DEVICE_NAME, &fops);
	if (device_major < 0) {
		pr_err("%s: failed to register device\n", DEVICE_NAME);
		__free_pages(reserved_page, get_mem_order(MEM_SIZE));
		return -ENOMEM;
	}

	pr_info("%s: loaded with major %i\n", DEVICE_NAME, device_major);
	return 0;
}

static void __exit mymodule_exit(void)
{
	unregister_chrdev(device_major, DEVICE_NAME);

	if (reserved_page)
		__free_pages(reserved_page, get_mem_order(MEM_SIZE));

	pr_info("%s: unloaded\n", DEVICE_NAME);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefano Munari");
MODULE_DESCRIPTION("Map physical contiguous memory");
