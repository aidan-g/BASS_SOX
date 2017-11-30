#ifdef _DEBUG
#include <stdio.h>
#endif

#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"
#include "resampler_lock.h"
#include "resampler_settings.h"
#include "resampler_soxr.h"

#define BUFFER_UPDATE_TIMEOUT 0

BASS_SOX_RESAMPLER* resampler_create() {
	return calloc(sizeof(BASS_SOX_RESAMPLER), 1);
}

BOOL resampler_free(BASS_SOX_RESAMPLER* resampler) {
	if (!resampler_soxr_free(resampler)) {
		return FALSE;
	}
	if (!resampler_buffer_free(resampler)) {
		return FALSE;
	}
	if (!resampler_settings_free(resampler)) {
		return FALSE;
	}
	if (!resampler_lock_free(resampler)) {
		return FALSE;
	}
	free(resampler);
	return TRUE;
}

BOOL ensure_resampler(BASS_SOX_RESAMPLER* resampler) {
	//If reload is requested then release the current resampler.
	if (resampler->reload) {
		resampler->ready = FALSE;
		resampler_soxr_free(resampler);
		resampler_buffer_free(resampler);
	}

	//If no resampler exists then create it.
	if (!resampler->soxr) {
		if (!resampler_soxr_create(resampler)) {
			return FALSE;
		}
		if (!resampler_buffer_create(resampler)) {
			return FALSE;
		}
		resampler->ready = TRUE;
	}
	return TRUE;
}

BOOL read_input_data(BASS_SOX_RESAMPLER* resampler) {
	BASS_SOX_RESAMPLER_BUFFER* buffer = resampler->buffer;
	buffer->input_buffer_length = BASS_ChannelGetData(
		resampler->source_channel,
		buffer->input_buffer,
		buffer->input_buffer_capacity
	);
	if (!buffer->input_buffer_length || buffer->input_buffer_length == BASS_STREAMPROC_END) {
#ifdef _DEBUG
		printf("Source channel has ended.\n");
#endif
		resampler->end = TRUE;
		return FALSE;
	}
	else if (buffer->input_buffer_length == BASS_ERR) {
#ifdef _DEBUG
		printf("Error reading from source channel.\n");
#endif
		resampler->end = TRUE;
		return FALSE;
	}
	if (resampler->end) {
		resampler->end = FALSE;
	}
	return TRUE;
}

BOOL read_output_data(BASS_SOX_RESAMPLER* resampler) {
	size_t frame_count;
	BASS_SOX_RESAMPLER_BUFFER* buffer = resampler->buffer;
	if (!buffer->input_buffer_length) {
		if (!read_input_data(resampler)) {
#ifdef _DEBUG
			printf("Could not read any input data.\n");
#endif
			return FALSE;
		}
	}
	if (resampler->end) {
		return FALSE;
	}
	resampler->soxr_error = soxr_process(
		resampler->soxr,
		buffer->input_buffer,
		buffer->input_buffer_length / resampler->input_frame_size,
		NULL,
		buffer->output_buffer,
		buffer->output_buffer_capacity / resampler->output_frame_size,
		&frame_count
	);
	if (resampler->soxr_error) {
#ifdef _DEBUG
		printf("Sox reported an error.\n");
#endif
		return FALSE;
	}
	buffer->input_buffer_length = 0;
	buffer->output_buffer_length = frame_count * resampler->output_frame_size;
	return TRUE;
}

BOOL read_playback_data(BASS_SOX_RESAMPLER* resampler) {
	DWORD remaining = 0;
	DWORD position = 0;
	DWORD attempts = 0;
	BASS_SOX_RESAMPLER_BUFFER* buffer = resampler->buffer;
	BASS_SOX_PLAYBACK_BUFFER* playback = buffer->playback;
	DWORD segment;
	if (!buffer->output_buffer_length) {
		if (!read_output_data(resampler)) {
			return FALSE;
		}
	}
	if (!playback) {
		//Playback buffer is disabled.
		return TRUE;
	}
	segment = playback->write_segment;
	remaining = buffer->output_buffer_length;
	do {
		for (; segment < playback->buffer->segment_count; segment++) {
			DWORD length;
			if (!ring_buffer_segment_length(playback->buffer, segment)) {
#ifdef _DEBUG
				printf("Populating ring buffer segment %d.\n", segment);
#endif
				if (remaining > playback->buffer->segment_capacity) {
					length = playback->buffer->segment_capacity;
				}
				else {
					length = remaining;
				}
				ring_buffer_write_segment(playback->buffer, segment, offset_buffer(buffer->output_buffer, position), length);
				if (playback->write_segment == (playback->buffer->segment_count - 1)) {
					playback->write_segment = 0;
				}
				else {
					playback->write_segment++;
				}
				remaining -= length;
				position += length;
				if (!remaining) {

					break;
				}
			}
			attempts++;
			if (attempts == playback->buffer->segment_count) {
				return FALSE;
			}
		}
		segment = 0;
	} while (remaining);
	buffer->output_buffer_length = 0;
	return TRUE;
}

BOOL write_playback_data(BASS_SOX_RESAMPLER* resampler, DWORD segment, DWORD length, void* buffer, DWORD* read_length) {
	BASS_SOX_PLAYBACK_BUFFER* playback = resampler->buffer->playback;
	DWORD segment_length = ring_buffer_segment_length(playback->buffer, segment);
	DWORD readable_length = segment_length - playback->read_position;
	if (readable_length > length) {
		*read_length = length;
	}
	else {
		*read_length = readable_length;
	}
	if (*read_length) {
		ring_buffer_read_segment_with_offset(playback->buffer, segment, playback->read_position, *read_length, buffer, read_length);
		playback->read_position += *read_length;
		if (playback->read_position == segment_length) {
#ifdef _DEBUG
			printf("Freeing ring buffer segment %d.\n", segment);
#endif
			ring_buffer_free_segment(playback->buffer, segment);
			if (playback->read_segment == (playback->buffer->segment_count - 1)) {
				playback->read_segment = 0;
			}
			else {
				playback->read_segment++;
			}
			playback->read_position = 0;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL resampler_populate(BASS_SOX_RESAMPLER* resampler) {
	DWORD success = FALSE;
	if (!resampler_lock_enter(resampler, IGNORE)) {
		return FALSE;
	}
	if (ensure_resampler(resampler)) {
		if (read_playback_data(resampler)) {
			success = TRUE;
		}
	}
	resampler_lock_exit(resampler);
	return success;
}

DWORD write_playback_data_direct(BASS_SOX_RESAMPLER* resampler, void* buffer, DWORD length) {
	DWORD remaining = length;
	DWORD position = 0;
	BASS_SOX_RESAMPLER_SETTINGS* settings = resampler->settings;
	do {
		DWORD output_remaining = resampler->buffer->output_buffer_length - resampler->buffer->output_buffer_position;
		if (output_remaining) {
			if (output_remaining > remaining) {
				output_remaining = remaining;
			}
			memcpy(
				offset_buffer(buffer, position),
				offset_buffer(resampler->buffer->output_buffer, resampler->buffer->output_buffer_position),
				output_remaining
			);
			remaining -= output_remaining;
			position += output_remaining;
			resampler->buffer->output_buffer_position += output_remaining;
			if (resampler->buffer->output_buffer_position == resampler->buffer->output_buffer_length) {
				resampler->buffer->output_buffer_length = 0;
				resampler->buffer->output_buffer_position = 0;
			}
		}
		if (remaining) {
			if (!settings->background) {
				resampler_populate(resampler);
				if (resampler->end) {
					if (settings->send_bass_streamproc_end) {
						return BASS_STREAMPROC_END;
					}
					else {
						return 0;
					}
				}
			}
			else {
#ifdef _DEBUG
				printf("Buffer underrun while reading output buffer.\n");
#endif
				goto buffer_underrun;
			}
		}
	} while (remaining);
buffer_underrun:
	return position;
}

DWORD CALLBACK resampler_proc(HSTREAM handle, void *buffer, DWORD length, void *user) {
	DWORD remaining = length;
	DWORD position = 0;
	DWORD read_length;
	DWORD attempts = 0;
	BASS_SOX_RESAMPLER* resampler = (BASS_SOX_RESAMPLER*)user;
	BASS_SOX_RESAMPLER_SETTINGS* settings = resampler->settings;
	BASS_SOX_PLAYBACK_BUFFER* playback;
	DWORD segment;
	if (!settings->background) {
		resampler_populate(resampler);
	}
	if (!resampler->ready) {
		return 0;
	}
	if (resampler->end) {
		if (settings->send_bass_streamproc_end) {
			return BASS_STREAMPROC_END;
		}
		else {
			return 0;
		}
	}
	playback = resampler->buffer->playback;
	if (!playback) {
		return write_playback_data_direct(resampler, buffer, length);
	}
	segment = playback->read_segment;
	do {
		for (; segment < playback->buffer->segment_count; segment++) {
			if (write_playback_data(resampler, segment, remaining, offset_buffer(buffer, position), &read_length)) {
				position += read_length;
				remaining -= read_length;
				if (!remaining) {
					break;
				}
			}
			attempts++;
			if (attempts == playback->buffer->segment_count) {
				if (!settings->background) {
					resampler_populate(resampler);
					if (resampler->end) {
						if (settings->send_bass_streamproc_end) {
							return BASS_STREAMPROC_END;
						}
						else {
							return 0;
						}
					}
					attempts = 0;
				}
				else {
#ifdef _DEBUG
					printf("Buffer underrun while reading playback buffer.\n");
#endif
					goto buffer_underrun;
				}
			}
		}
		segment = 0;
	} while (remaining);
buffer_underrun:
	return position;
}