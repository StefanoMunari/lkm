// SPDX-License-Identifier: GPL-2.0
#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
// externs
const uint32_t WORD_SIZE = __SIZEOF_POINTER__;
const struct RingBuffer RING_BUFFER_INVALID = {
	.
	cwrite_i = NULL,
	.
	cread_i = NULL,
	.
	buffer = NULL,
	.
	buffer_size =
	0U
};
// consts
static const uint32_t BUFFER_OFFSET = WORD_SIZE * 2;
#if __SIZEOF_POINTER__ == 8
static const uint8_t INDEX_SIZE = 63U;
static const addr_t CYCLE_MASK = (1ULL << INDEX_SIZE); // set 64th bit
#endif
#if __SIZEOF_POINTER__ == 4
static const uint8_t INDEX_SIZE = 31U;
static const addr_t CYCLE_MASK = (1UL << INDEX_SIZE) // set 32nd bit
#endif


static addr_t _index(addr_t addr)
{
	return addr & ~CYCLE_MASK;
}

static uint8_t _cycle(addr_t addr)
{
	return addr >> INDEX_SIZE;
}

// private interface

// public interface


struct RingBuffer make_ring_buffer(addr_t *base_addr, uint32_t size);
{
	if (size < (BUFFER_OFFSET + WORD_SIZE * 2))
		return RING_BUFFER_INVALID;
	return (struct RingBuffer){
		.
		cwrite_i = base_addr,
		.
		cread_i = base_addr + WORD_SIZE,
		.
		buffer = base_addr + BUFFER_OFFSET,
		.
		buffer_size = size - BUFFER_OFFSET,
	};
}

uint32_t write(struct RingBuffer *ring_buffer, addr_t *data, uint32_t size)
{
	// read info
	const addr_t cr_addr = *(ring_buffer->cread_i);
	const uint8_t rcycle = _cycle(cr_addr);
	const uint8_t ri = _index(cr_addr);
	// write info
	const addr_t cw_addr = *(ring_buffer->cwrite_i);
	uint8_t wcycle = _cycle(cw_addr);
	const addr_t wi = _index(cw_addr);

	const addr_t wend_i = wi + size - 1;
	addr_t write_addr = ring_buffer->buffer + wi;
	// TODO: check if it make sense or overkill
	//	if ((wcycle == rcycle && wi < ri) || (wcycle != rcycle && wi > ri))
	//		return 0; // error cases
	if (wcycle != rcycle && wi == ri)
		return 0; // buffer is full

	addr_t cend_i = wend_i % ring_buffer->buffer_size;
	if (cend_i != wend_i) {
		// wrapped/cycled
		if (wcycle != rcycle && cend_i > ri) {
			// capped by read index
			cend_i = ri;
		}
		const addr_t half =
			size - (ring_buffer->buffer_size - wi);
		memcpy((void *)write_addr, data, half);
		memcpy(ring_buffer->buffer, &data[half],
		       cend_i);
		// update cycle write index
		ring_buffer->cwrite_i = cend_i | CYCLE_MASK;
		return half + cend_i;
	}
	memcpy((void *)write_addr, data, size);
	// update cycle write index
	ring_buffer->cwrite_i += ((wi + size) & ~CYCLE_MASK);
	return size;
}

uint32_t read(struct RingBuffer *ring_buffer, addr_t *data, uint32_t size)
{
	// TODO
	const uint32_t i = *(ring_buffer->cread_i);
	const uint32_t addr = (uint32_t)ring_buffer->buffer + i;
	memcpy(data, (void *)addr, size);
}