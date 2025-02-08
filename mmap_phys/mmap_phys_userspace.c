#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static const size_t BUFFER_SIZE = 256;
static const char DEVICE[] = "/dev/mmap_phys_dev";

int main()
{
	size_t const PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
	size_t const MAPPED_MEM_SIZE = PAGE_SIZE;
	off_t const MAPPED_MEM_OFFSET = MAPPED_MEM_SIZE;

	int fd = open(DEVICE, O_RDWR);
	if (fd < 0) {
		char err_buffer[BUFFER_SIZE];
		snprintf(err_buffer, BUFFER_SIZE,
		         "Failed to open device:%s",
		         DEVICE);
		perror(err_buffer);
		return 1;
	}
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

	if (mapped == MAP_FAILED) {
		char err_buffer[BUFFER_SIZE];
		snprintf(err_buffer, BUFFER_SIZE,
		         "Failed map memory:%s",
		         DEVICE);
		perror(err_buffer);
		return 1;
	}

	printf("Memory mapped at address %p\n", mapped);

	// Example access to mapped memory
	((char *)mapped)[0] = 'A';
	printf("Memory mapped wtire: %s\n", (char *)mapped);

	munmap(mapped, MAPPED_MEM_SIZE);
	close(fd);

	return 0;
}
