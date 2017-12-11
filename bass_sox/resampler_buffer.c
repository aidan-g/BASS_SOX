#include <stdio.h>
#include "resampler_buffer.h"

#define DEFAULT_BUFFER_LENGTH 1

size_t get_buffer_length(BASS_SOX_RESAMPLER* resampler) {
	BASS_SOX_RESAMPLER_SETTINGS* settings = resampler->settings;
	if (settings->buffer_length) {
		return settings->buffer_length;
	}
	else {
		return DEFAULT_BUFFER_LENGTH;
	}
}

BOOL resampler_buffer_create(BASS_SOX_RESAMPLER* resampler) {
	size_t buffer_length = get_buffer_length(resampler);
	if (resampler->buffer) {
		resampler_buffer_free(resampler);
	}
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

BOOL resampler_buffer_clear(BASS_SOX_RESAMPLER* resampler) {
	if (!resampler->buffer) {
		return FALSE;
	}
	resampler->buffer->input_buffer_length = 0;
	resampler->buffer->output_buffer_length = 0;
	if (resampler->buffer->playback) {
		resampler->buffer->playback->read_position = 0;
		resampler->buffer->playback->read_segment = 0;
		resampler->buffer->playback->write_position = 0;
		resampler->buffer->playback->write_segment = 0;
		ring_buffer_clear(resampler->buffer->playback->buffer);
	}
	return TRUE;
}

BOOL resampler_buffer_free(BASS_SOX_RESAMPLER* resampler) {
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
		resampler->buffer = NULL;
	}
	return TRUE;
}

BOOL resampler_buffer_length(BASS_SOX_RESAMPLER* resampler, DWORD* length) {
	DWORD a;
	BASS_SOX_PLAYBACK_BUFFER* buffer;
	if (!resampler->buffer) {
		return FALSE;
	}
	buffer = resampler->buffer->playback;
	if (buffer) {
		*length = 0;
		for (a = 0; a < buffer->buffer->segment_count; a++) {
			if (ring_buffer_segment_length(buffer->buffer, a)) {
				(*length)++;
			}
		}
	}
	else {
		if (resampler->buffer->output_buffer_length) {
			*length = 1;
		}
		else {
			*length = 0;
		}
	}
	return TRUE;
}