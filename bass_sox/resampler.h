#ifndef RESAMPLER
#define RESAMPLER

#include "bass/bass.h"
#include "soxr/soxr.h"

#define BASS_ERR -1
#define BASS_ERR_END 0xFFFFFFFF

typedef struct {
	int input_rate;
	int output_rate;
	int channels;
	HSTREAM source_channel;
	HSTREAM output_channel;
	DWORD bass_error;
	size_t ratio;
	unsigned int threads;
	size_t buffer_length;
	size_t input_sample_size;
	size_t input_frame_size;
	size_t output_sample_size;
	size_t output_frame_size;
	DWORD input_buffer_length;
	DWORD output_buffer_length;
	void* input_buffer;
	void* output_buffer;
	DWORD output_length;
	DWORD output_position;
	unsigned long quality;
	unsigned long phase;
	BOOL steep_filter;
	BOOL allow_aliasing;
	soxr_t soxr;
	soxr_error_t soxr_error;
	BOOL reload;
} BASS_SOX_RESAMPLER;

BOOL populate_resampler(BASS_SOX_RESAMPLER* resampler);

DWORD CALLBACK resampler_proc(HSTREAM handle, void *buffer, DWORD length, void *user);

#endif