#include <stdio.h>
#include "resampler_buffer.h"

#define DEFAULT_BUFFER_LENGTH 1

size_t get_buffer_length(BASS_SOX_RESAMPLER* resampler) {
	if (resampler->buffer_length) {
		return resampler->buffer_length;
	}
	else {
		return DEFAULT_BUFFER_LENGTH;
	}
}

BOOL alloc_resampler_buffers(BASS_SOX_RESAMPLER* resampler) {
	size_t buffer_length = get_buffer_length(resampler);
	resampler->buffer = calloc(sizeof(BASS_SOX_RESAMPLER_BUFFER), 1);
	resampler->buffer->input_buffer_capacity = resampler->input_rate * resampler->input_frame_size;
	resampler->buffer->output_buffer_capacity = resampler->output_rate * resampler->output_frame_size;
	resampler->buffer->input_buffer = malloc(resampler->buffer->input_buffer_capacity);
	resampler->buffer->output_buffer = malloc(resampler->buffer->output_buffer_capacity);
	if (buffer_length > 1) {
		resampler->buffer->playback = calloc(sizeof(BASS_SOX_PLAYBACK_BUFFER), 1);
		resampler->buffer->playback->buffer = ring_buffer_create(resampler->buffer->output_buffer_capacity, buffer_length);
	}
	return TRUE;
}

BOOL release_resampler_buffers(BASS_SOX_RESAMPLER* resampler) {
	if (resampler->buffer) {
		if (resampler->buffer->playback) {
			if (resampler->buffer->playback->buffer) {
				ring_buffer_free(resampler->buffer->playback->buffer);
			}
			free(resampler->buffer->playback);
		}
		if (resampler->buffer->input_buffer) {
			free(resampler->buffer->input_buffer);
		}
		if (resampler->buffer->output_buffer) {
			free(resampler->buffer->output_buffer);
		}
		free(resampler->buffer);
	}
	return TRUE;
}