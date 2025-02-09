
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

// Architecture addressing:
// if none supported -> dont compile
#if __SIZEOF_POINTER__ == 8
typedef uint64_t addr_t;
#endif
#if __SIZEOF_POINTER__ == 4
typedef uint32_t addr_t;
#endif

struct RingBuffer {
	// ptr to cycle write index
	addr_t *cwrite_i;
	// ptr to cycle read index
	addr_t *cread_i;
	// ptr to data
	addr_t *buffer;
	// size of buffer in bytes
	addr_t buffer_size;
} __attribute__((aligned(sizeof(addr_t))));

extern const uint32_t WORD_SIZE;
extern const struct RingBuffer RING_BUFFER_INVALID;

// creates a ring buffer from a chunk of memory defined by base_addr + size
// return RING_BUFFER_INVALID (buffer_size = 0) if parameters are unusable
// note: use ring buffer as input (*this) obj to related functions,
// do not touch its internals
struct RingBuffer make_ring_buffer(addr_t *base_addr, uint32_t size);

// write size data into ring_buffer from cwrite_i position
// return number of bytes written
uint32_t write(struct RingBuffer *ring_buffer, addr_t *data, uint32_t size);

// read size data from ring_buffer cread_i position
// return number of bytes read
uint32_t read(struct RingBuffer *ring_buffer, addr_t *data, uint32_t size);


#endif //RING_BUFFER_H
