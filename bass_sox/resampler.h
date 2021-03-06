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
	size_t output_buffer_position;
	BASS_SOX_PLAYBACK_BUFFER* playback;
} BASS_SOX_RESAMPLER_BUFFER;

typedef struct {
	unsigned long quality;
	unsigned long phase;
	BOOL steep_filter;
	BOOL allow_aliasing;
	size_t buffer_length;
	unsigned int threads;
	BOOL background;
	BOOL keep_alive;
} BASS_SOX_RESAMPLER_SETTINGS;

typedef struct {
	int input_rate;
	int output_rate;
	int channels;
	HSTREAM source_channel;
	HSTREAM output_channel;
	BOOL end;
	size_t input_sample_size;
	size_t input_frame_size;
	size_t output_sample_size;
	size_t output_frame_size;
	BASS_SOX_RESAMPLER_BUFFER* buffer;
	BASS_SOX_RESAMPLER_SETTINGS* settings;
	soxr_t soxr;
	soxr_error_t soxr_error;
	BOOL reload;
	HANDLE lock;
	HANDLE thread;
	volatile BOOL shutdown;
	volatile BOOL ready;
} BASS_SOX_RESAMPLER;

BASS_SOX_RESAMPLER* resampler_create();

BOOL resampler_free(BASS_SOX_RESAMPLER* resampler);

BOOL resampler_populate(BASS_SOX_RESAMPLER* resampler);

__declspec(dllexport)
DWORD CALLBACK resampler_proc(HSTREAM handle, void *buffer, DWORD length, void *user);

#endif