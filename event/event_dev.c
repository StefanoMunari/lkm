// SPDX-License-Identifier: GPL-2.0
/*
 * Simple notification mechanism: kernel <-> userspace
 * No payload in notification messages, just the notification event.
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/poll.h>

static const char DEVICE_NAME[] = "event_dev";
static unsigned int event_major;
static int device_open_count = 0;

static int event_open(struct inode *inode, struct file *file)
{
	if (device_open_count)
		return -EBUSY;
	++device_open_count;
	pr_info("%s: open\n", DEVICE_NAME);
	return 0;
}

static int event_release(struct inode *inode, struct file *file)
{
	--device_open_count;
	pr_info("%s: close\n", DEVICE_NAME);
	return 0;
}

// send signal to userspace
static ssize_t event_send(struct file *file, char __user *user_buffer,
                          size_t length, loff_t *offset)
{
	static const uint8_t event = 0xFF;
	// copy_to_user: must pass something (1B), otherwise ignored
	if (copy_to_user(user_buffer, &event, sizeof(event))) {
		return -EFAULT;
	}
	pr_info("%s: signal{kernel -> usr}\n", DEVICE_NAME);
	return 0;
}

// recv signal from userspace
static ssize_t event_recv(struct file *file, const char __user *user_buffer,
                          size_t length, loff_t *offset)
{
	pr_info("%s: signal{usr -> kernel}\n", DEVICE_NAME);
	//process_recvd_data
	return 0;
}

static struct file_operations event_fops = {
	.open = event_open,
	.read = event_send,
	.write = event_recv,
	.release = event_release,
};

static int __init event_init(void)
{
	// register as character device
	event_major = register_chrdev(0, DEVICE_NAME, &event_fops);
	pr_info("%s: loaded with major %d\n", DEVICE_NAME, event_major);
	return 0;
}

static void __exit event_exit(void)
{
	unregister_chrdev(event_major, DEVICE_NAME);
	pr_info("%s: unloaded\n", DEVICE_NAME);
}

module_init(event_init);

module_exit(event_exit);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Stefano Munari");

MODULE_DESCRIPTION("Event Device Driver");