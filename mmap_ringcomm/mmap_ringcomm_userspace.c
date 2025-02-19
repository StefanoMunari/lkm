// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "ring_buffer/ring_buffer.h"

static const size_t BUFFER_SIZE = 256;
static const char DEVICE[] = "/dev/mmap_ringbuffer_dev";

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
	struct RingBuffer ring_buffer;
	return 0;
}
