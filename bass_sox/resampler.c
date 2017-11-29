#ifdef _DEBUG
#include <stdio.h>
#endif
#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"
#include "resampler_lock.h"

#define SOXR_DEFAULT_THREADS 1
#define SOXR_DEFAULT_QUALITY SOXR_VHQ
#define SOXR_DEFAULT_PHASE SOXR_LINEAR_PHASE

#define BUFFER_UPDATE_TIMEOUT 0

//Get the corresponding soxr_datatype_t for the specified sample size (either short or float).
soxr_datatype_t soxr_datatype(size_t sample_size) {
	if (sample_size == sizeof(short)) {
		return SOXR_INT16_I;
	}
	if (sample_size == sizeof(float)) {
		return SOXR_FLOAT32_I;
	}
	return 0;
}

soxr_quality_spec_t get_quality_spec(BASS_SOX_RESAMPLER* resampler) {
	unsigned long recipe = 0;
	unsigned long flags = 0;
	if (resampler->quality) {
		recipe |= resampler->quality;
	}
	else {
		recipe |= SOXR_DEFAULT_QUALITY;
	}
	if (resampler->phase) {
		recipe |= resampler->phase;
	}
	else {
		recipe |= SOXR_DEFAULT_PHASE;
	}
	if (resampler->steep_filter) {
		recipe |= SOXR_STEEP_FILTER;
	}
	if (resampler->allow_aliasing) {
		//No longer implemented.
	}
	return soxr_quality_spec(recipe, flags);
}

soxr_runtime_spec_t get_runtime_spec(BASS_SOX_RESAMPLER* resampler) {
	unsigned int threads = 0;
	if (resampler->threads) {
		threads = resampler->threads;
	}
	else {
		threads = SOXR_DEFAULT_THREADS;
	}
	return soxr_runtime_spec(threads);
}

//Construct a soxr_t for the specified resampler configuration.
BOOL create_soxr_resampler(BASS_SOX_RESAMPLER* resampler) {

	soxr_io_spec_t io_spec = soxr_io_spec(
		soxr_datatype(resampler->input_sample_size),
		soxr_datatype(resampler->output_sample_size));
	soxr_quality_spec_t quality_spec = get_quality_spec(resampler);
	soxr_runtime_spec_t runtime_spec = get_runtime_spec(resampler);

	if (resampler->soxr) {
		release_soxr(resampler);
	}

	resampler->soxr = soxr_create(
		resampler->input_rate,
		resampler->output_rate,
		resampler->channels,
		&resampler->soxr_error,
		&io_spec,
		&quality_spec,
		&runtime_spec);

	if (!resampler->soxr) {
		return FALSE;
	}

	if (!alloc_resampler_buffers(resampler)) {
		release_soxr(resampler);
		return FALSE;
	}

	return TRUE;
}

BOOL ensure_resampler(BASS_SOX_RESAMPLER* resampler) {
	//If reload is requested then release the current resampler.
	if (resampler->reload) {
		resampler->ready = FALSE;
		release_soxr(resampler);
		release_resampler_buffers(resampler);
	}

	//If no resampler exists then create it.
	if (!resampler->soxr) {
		if (!create_soxr_resampler(resampler)) {
#ifdef _DEBUG
			printf("Failed to create soxr resampler.\n");
#endif
			return FALSE;
		}
		resampler->ready = TRUE;
	}
	return TRUE;
}

BOOL read_input_data(BASS_SOX_RESAMPLER* resampler) {
	resampler->buffer->input_buffer_length = BASS_ChannelGetData(
		resampler->source_channel,
		resampler->buffer->input_buffer,
		resampler->buffer->input_buffer_capacity
	);
	if (!resampler->buffer->input_buffer_length || resampler->buffer->input_buffer_length == BASS_STREAMPROC_END) {
#ifdef _DEBUG
		printf("Source channel has ended.\n");
#endif
		resampler->end = TRUE;
		return FALSE;
	}
	else if (resampler->buffer->input_buffer_length == BASS_ERR) {
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
	if (!resampler->buffer->input_buffer_length) {
		if (!read_input_data(resampler)) {
#ifdef _DEBUG
			printf("Could not read any input data.\n");
#endif
			return FALSE;
		}
	}
	resampler->soxr_error = soxr_process(
		resampler->soxr,
		resampler->buffer->input_buffer,
		resampler->buffer->input_buffer_length / resampler->input_frame_size,
		NULL,
		resampler->buffer->output_buffer,
		resampler->buffer->output_buffer_capacity / resampler->output_frame_size,
		&frame_count
	);
	if (resampler->soxr_error) {
#ifdef _DEBUG
		printf("Sox reported an error.\n");
#endif
		return FALSE;
	}
	resampler->buffer->input_buffer_length = 0;
	resampler->buffer->output_buffer_length = frame_count * resampler->output_frame_size;
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

BOOL populate_resampler(BASS_SOX_RESAMPLER* resampler) {
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
			if (!resampler->background) {
				populate_resampler(resampler);
				if (resampler->end) {
					if (resampler->send_bass_streamproc_end) {
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
	BASS_SOX_PLAYBACK_BUFFER* playback;
	DWORD segment;
	if (!resampler->background) {
		populate_resampler(resampler);
	}
	if (!resampler->ready) {
		return 0;
	}
	if (resampler->end) {
		if (resampler->send_bass_streamproc_end) {
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
				if (!resampler->background) {
					populate_resampler(resampler);
					if (resampler->end) {
						if (resampler->send_bass_streamproc_end) {
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