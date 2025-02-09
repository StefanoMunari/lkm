// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

static const size_t BUFFER_SIZE = 256;
static const char DEVICE[] = "/dev/mmap_phys_dev";

static int handle_err(const char *msg)
{
	static const int ERR = -1;
	char err_buffer[BUFFER_SIZE];
	snprintf(err_buffer, BUFFER_SIZE,
	         "%s:%s",
	         msg, DEVICE);
	perror(err_buffer);
	return ERR;
}

int main()
{
	size_t const PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
	size_t const MAPPED_MEM_SIZE = PAGE_SIZE;
	off_t const MAPPED_MEM_OFFSET = MAPPED_MEM_SIZE;
	// get device related file descriptor
	int fd = open(DEVICE, O_RDWR);
	if (fd < 0)
		return handle_err("Failed to open device");
	// syscall:
	// void *mmap(void addr, size_t length, int prot, int flags, int fd, off_t offset)
	// addr = NULL: kernel choose page aligned addr
	// lenght: size of the mapping in bytes
	// prot: protection flags of the mapped mem (read | write)
	// flags: MAP_SHARED implies updates to mmap visible to other processes
	//	+ carried to the underlying file (no copy-on-write)
	//	shared between userspace processes and kernel
	// fd: the file descriptor associated with mmap
	// offset: userspace mmap initialized with MEM_SIZE from OFFSET
	void *mapped = mmap(NULL, MAPPED_MEM_SIZE, PROT_READ | PROT_WRITE,
	                    MAP_SHARED,
	                    fd, MAPPED_MEM_OFFSET);

	if (mapped == MAP_FAILED)
		return handle_err("Failed to map memory");

	printf("Memory mapped at address: 0x%lx, size: 0x%lx\n",
	       (unsigned long *)mapped, MAPPED_MEM_SIZE);

	char message[BUFFER_SIZE];
	memset(message, 0x00, BUFFER_SIZE);
	for (size_t i = 0; (i < BUFFER_SIZE) && (0x30 + i < 0x7B); i++) {
		message[i] = 0x030 + i;
	}
	// write directly to remapped physical memory
	printf("Writing message: %s\n", message);
	memcpy(mapped, message, BUFFER_SIZE);
	// reset buffer
	memset(message, 0x00, BUFFER_SIZE);
	// read from remapped mem
	memcpy(message, mapped, BUFFER_SIZE);
	printf("Memory read usr: %s\n", message);
	// syscall:
	// int munmap(void *addr, size_t length);
	// rm mappings for the specified address range
	// causes further references to addresses to reference invalid memory.
	// Region automatically unmapped when the process is terminated.
	// *Closing the file descriptor does not unmap the region*
	// ret: 0 on success, -1 on failure
	if (munmap(mapped, MAPPED_MEM_SIZE))
		return handle_err("Failed to unmap memory");
	if (close(fd))
		return handle_err("Failed to close device");

	return 0;
}
