#include <stdlib.h>
#include "ring_buffer.h"

RING_BUFFER* ring_buffer_create(size_t segment_capacity, size_t segment_count) {
	DWORD current_segment;
	RING_BUFFER* ring_buffer = calloc(sizeof(RING_BUFFER), 1);
	ring_buffer->segment_capacity = segment_capacity;
	ring_buffer->segment_count = segment_count;
	ring_buffer->buffer = malloc(segment_count * segment_capacity);
	ring_buffer->segments = malloc(sizeof(RING_BUFFER_SEGMENT*) * segment_count);
	for (current_segment = 0; current_segment < segment_count; current_segment++) {
		ring_buffer->segments[current_segment] = calloc(sizeof(RING_BUFFER_SEGMENT), 1);
	}
	return ring_buffer;
}

void ring_buffer_free(RING_BUFFER* ring_buffer) {
	DWORD current_segment;
	if (ring_buffer->buffer) {
		free(ring_buffer->buffer);
	}
	if (ring_buffer->segments) {
		for (current_segment = 0; current_segment < ring_buffer->segment_count; current_segment++) {
			if (ring_buffer->segments[current_segment]) {
				free(ring_buffer->segments[current_segment]);
			}
		}
		free(ring_buffer->segments);
	}
	free(ring_buffer);
}

void* offset_buffer(void* buffer, DWORD position) {
	return (BYTE*)buffer + position;
}

void* ring_buffer_offset_buffer_by_segment(RING_BUFFER* ring_buffer, DWORD segment) {
	return offset_buffer(ring_buffer->buffer, ring_buffer->segment_capacity * segment);
}

void* ring_buffer_offset_buffer_by_segment_and_byte(RING_BUFFER* ring_buffer, DWORD segment, DWORD position) {
	return offset_buffer(ring_buffer->buffer, (ring_buffer->segment_capacity * segment) + position);
}

void ring_buffer_read_segment(RING_BUFFER* ring_buffer, DWORD segment, DWORD capacity, void* buffer, DWORD* length) {
	RING_BUFFER_SEGMENT* ring_buffer_segment = ring_buffer->segments[segment];
	if (ring_buffer_segment->length < capacity) {
		capacity = ring_buffer_segment->length;
	}
	memcpy(
		buffer,
		ring_buffer_offset_buffer_by_segment(ring_buffer, segment),
		capacity
	);
	*length = capacity;
}

void ring_buffer_read_segment_with_offset(RING_BUFFER* ring_buffer, DWORD segment, DWORD position, DWORD capacity, void* buffer, DWORD* length) {
	RING_BUFFER_SEGMENT* ring_buffer_segment = ring_buffer->segments[segment];
	DWORD remaining = ring_buffer_segment->length - position;
	if (remaining < capacity) {
		capacity = remaining;
	}
	memcpy(
		buffer,
		ring_buffer_offset_buffer_by_segment_and_byte(ring_buffer, segment, position),
		capacity
	);
	*length = capacity;
}

void ring_buffer_write_segment(RING_BUFFER* ring_buffer, DWORD segment, void* buffer, DWORD length) {
	RING_BUFFER_SEGMENT* ring_buffer_segment = ring_buffer->segments[segment];
	memcpy(
		ring_buffer_offset_buffer_by_segment(ring_buffer, segment),
		buffer,
		length
	);
	ring_buffer_segment->length = length;
}

void ring_buffer_free_segment(RING_BUFFER* ring_buffer, DWORD segment) {
	RING_BUFFER_SEGMENT* ring_buffer_segment = ring_buffer->segments[segment];
	ring_buffer_segment->length = 0;
}

DWORD ring_buffer_segment_length(RING_BUFFER* ring_buffer, DWORD segment) {
	RING_BUFFER_SEGMENT* ring_buffer_segment = ring_buffer->segments[segment];
	return ring_buffer_segment->length;
}