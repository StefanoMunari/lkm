// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "ring_buffer/ring_buffer.h"

static const size_t BUFFER_SIZE = 256;
static const char DEVICE[] = "/dev/mmap_ringcomm_dev";
static const int ERR = -1;

static int handle_err(const char *msg)
{
	char err_buffer[BUFFER_SIZE];
	snprintf(err_buffer, BUFFER_SIZE,
	         "%s:%s",
	         msg, DEVICE);
	perror(err_buffer);
	return ERR;
}

static void *get_mmap(int fd, size_t size, off_t offset)
{
	void *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE,
	                    MAP_SHARED,
	                    fd, offset);

	if (mapped == MAP_FAILED) {
		handle_err("Failed to map memory");
		return NULL;
	}

	printf("Memory mapped at address: 0x%lx, size: 0x%lx\n",
	       (unsigned long *)mapped, size);

	return mapped;
}

static void make_message(uint8_t *buffer, size_t size)
{
	for (size_t i = 0; i < size; ++i) {
		buffer[i] = 0x41 + ((0x41 + i) % 0x7A);
	}
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

	void *mapped = get_mmap(fd, MAPPED_MEM_SIZE, MAPPED_MEM_OFFSET);
	if (mapped == NULL)
		return ERR;

	struct RingBuffer ring_buffer = ring_buffer_make_linear(
		mapped, MAPPED_MEM_SIZE);
	uint8_t buffer[BUFFER_SIZE];
	make_message(buffer, BUFFER_SIZE);

	// write directly to remapped physical memory using ring_buffer
	printf("Writing using ring buffer\n");
	int32_t written = ring_buffer_write(&ring_buffer, buffer, BUFFER_SIZE);
	if (written < 0)
		return handle_err("Failed ring buffer write");
	printf("Memory written usr: %i\n", written);

	printf("Notify kernelspace\n");
	uint8_t event = 0xAA;
	if (write(fd, &event, sizeof(event)) < 0)
		return handle_err("Failed to notify kernel");

	printf("Blocking read: waiting from kernelspace");
	if (read(fd, &event, sizeof(event)) < 0)
		return handle_err("Failed read form device");
	printf("Notified from kernelspace");

	memset(buffer, 0x00, BUFFER_SIZE);
	int32_t read = ring_buffer_read(&ring_buffer, buffer, BUFFER_SIZE);
	if (read < 0)
		return handle_err("Failed ring buffer read");
	printf("Memory read usr: %i\n", read);

	if (munmap(mapped, MAPPED_MEM_SIZE))
		return handle_err("Failed to unmap memory");
	if (close(fd))
		return handle_err("Failed to close device");

	return 0;
}
