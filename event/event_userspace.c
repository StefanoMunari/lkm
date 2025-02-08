#include <stdio.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>

static const size_t BUFFER_SIZE = 256;
static const char DEVICE[] = "/dev/event_dev";

int main()
{
	int fd = open(DEVICE, O_RDWR);

	if (fd < 0) {
		char err_buffer[BUFFER_SIZE];
		snprintf(err_buffer, BUFFER_SIZE,
		         "Failed to open device:%s",
		         DEVICE);
		perror(err_buffer);
		return 1;
	}

	printf("Processing data...\n");
	// Write some data to trigger kernel-side
	uint8_t event = 0xAA;
	int result = write(fd, &event, sizeof(event));
	// Trigger event_recv in kernel
	if (result == -1) {
		perror("Failed to write to device");
		close(fd);
		return 1;
	}
	printf("Kernelspace Notified\n");

	// Wait for notification from the kernel
	read(fd, &event, sizeof(event));
	printf("Notified from kernelspace");

	close(fd);
	return 0;
}