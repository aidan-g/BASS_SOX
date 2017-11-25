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
	size_t ratio;
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
	soxr_t soxr;
	soxr_error_t soxr_error;
} BASS_SOX_RESAMPLER;

DWORD CALLBACK resampler_proc(HSTREAM handle, void *buffer, DWORD length, void *user);

#endif