#ifndef RESAMPLER
#define RESAMPLER

#include "bass/bass.h"
#include "soxr/soxr.h"

#include "ring_buffer.h"

#define BASS_ERR -1

#define MAX_BUFFER_LENGTH 10

typedef struct {
	RING_BUFFER* buffer;
	DWORD read_segment;
	DWORD write_segment;
	DWORD read_position;
	DWORD write_position;
} BASS_SOX_PLAYBACK_BUFFER;

typedef struct {
	void* input_buffer;
	size_t input_buffer_length;
	size_t input_buffer_capacity;
	void* output_buffer;
	size_t output_buffer_length;
	size_t output_buffer_capacity;
	BASS_SOX_PLAYBACK_BUFFER* playback;
} BASS_SOX_RESAMPLER_BUFFER;

typedef struct {
	int input_rate;
	int output_rate;
	int channels;
	HSTREAM source_channel;
	HSTREAM output_channel;
	BOOL end;
	DWORD bass_error;
	size_t ratio;
	size_t input_sample_size;
	size_t input_frame_size;
	size_t output_sample_size;
	size_t output_frame_size;
	BASS_SOX_RESAMPLER_BUFFER* buffer;
	unsigned long quality;
	unsigned long phase;
	BOOL steep_filter;
	BOOL allow_aliasing;
	size_t buffer_length;
	unsigned int threads;
	BOOL background;
	BOOL send_bass_streamproc_end;
	soxr_t soxr;
	soxr_error_t soxr_error;
	BOOL reload;
	HANDLE lock;
	volatile BOOL ready;
} BASS_SOX_RESAMPLER;

BOOL populate_resampler(BASS_SOX_RESAMPLER* resampler);

DWORD CALLBACK resampler_proc(HSTREAM handle, void *buffer, DWORD length, void *user);

#endif