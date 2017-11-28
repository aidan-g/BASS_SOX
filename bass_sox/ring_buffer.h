#include <windows.h>

typedef struct {
	size_t length;
} RING_BUFFER_SEGMENT;

typedef struct {
	void* buffer;
	size_t segment_count;
	size_t segment_capacity;
	RING_BUFFER_SEGMENT**  segments;
} RING_BUFFER;

RING_BUFFER* ring_buffer_create(size_t segment_capacity, size_t segment_count);

void ring_buffer_free(RING_BUFFER* ring_buffer);

void* offset_buffer(void* buffer, DWORD position);

void ring_buffer_read_segment(RING_BUFFER* ring_buffer, DWORD segment, DWORD capacity, void* buffer, DWORD* length);

void ring_buffer_read_segment_with_offset(RING_BUFFER* ring_buffer, DWORD segment, DWORD position, DWORD capacity, void* buffer, DWORD* length);

void ring_buffer_write_segment(RING_BUFFER* ring_buffer, DWORD segment, void* buffer, DWORD length);

void ring_buffer_free_segment(RING_BUFFER* ring_buffer, DWORD segment);

DWORD ring_buffer_segment_length(RING_BUFFER* ring_buffer, DWORD segment);