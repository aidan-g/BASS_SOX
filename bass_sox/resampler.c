#include <stdio.h>
#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"

#define SOXR_DEFAULT_THREADS 1
#define SOXR_DEFAULT_QUALITY SOXR_VHQ
#define SOXR_DEFAULT_PHASE SOXR_LINEAR_PHASE

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
		release_soxr(resampler);
		release_resampler_buffers(resampler);
	}

	//If no resampler exists then create it.
	if (!resampler->soxr) {
		if (!create_soxr_resampler(resampler)) {
#ifdef DEBUG
			printf("Failed to create soxr resampler.\n");
#endif
			return FALSE;
		}
	}
	return TRUE;
}

BOOL get_input_channel_data(BASS_SOX_RESAMPLER* resampler, DWORD* length) {
	*length = BASS_ChannelGetData(
		resampler->source_channel,
		resampler->input_buffer,
		resampler->input_buffer_length);

	if (*length == BASS_ERR) {
		resampler->bass_error = BASS_ErrorGetCode();
#ifdef DEBUG
		printf("Failed to read from source channel: %d\n", resampler->bass_error);
#endif
		return FALSE;
	}

	if (*length == BASS_ERR_END) {
#ifdef DEBUG
		printf("Failed to read from source channel: Ended.\n");
#endif
		resampler->bass_error = BASS_STREAMPROC_END;
		return FALSE;
	}

	return TRUE;
	}

DWORD get_output_channel_data(BASS_SOX_RESAMPLER* resampler, DWORD length) {

	size_t resample_input_read_length;
	size_t resample_output_written_length;
	//Calculate how many frames can be read/written.
	size_t input_frames = length / resampler->input_frame_size;
	size_t output_frames = resampler->output_buffer_length / resampler->output_frame_size;

	//Invoke soxr to resample the input data and check for errors.
	resampler->soxr_error = soxr_process(
		resampler->soxr,
		resampler->input_buffer,
		input_frames,
		&resample_input_read_length,
		resampler->output_buffer,
		output_frames,
		&resample_output_written_length);

	if (resampler->soxr_error) {
#ifdef DEBUG
		printf("Failed to resample data: %s.\n", resampler->soxr_error);
#endif
		return FALSE;
	}

	if (resample_input_read_length < input_frames) {
#ifdef DEBUG
		printf("Input was not completed, buffer is too small?\n");
#endif
		return FALSE;
	}

	//Update the output buffer length.
	resampler->output_length = resample_output_written_length * resampler->output_frame_size;

#ifdef DEBUG
	printf("Resampled %d frames (%d bytes) into %d frames (%d bytes) at a ratio of 1 to %d.\n",
		source_channel_read_length,
		input_frames,
		resampler->output_length,
		resample_output_written_length,
		resampler->ratio);
#endif

	return TRUE;
	}

BOOL populate_resampler(BASS_SOX_RESAMPLER* resampler) {
	DWORD length;

	//Ensure the resampler (and buffers) are available.
	if (!ensure_resampler(resampler)) {
		//Failed to create resampler or allocate buffers.
		return FALSE;
	}

	//Read the source data to be resampled, check for read errors.
	if (!get_input_channel_data(resampler, &length)) {
		//Failed to read input data for some reason.
		return FALSE;
	}

	if (!get_output_channel_data(resampler, length)) {
		//Failed to write the output data for some reason.
		return FALSE;
	}
	return TRUE;
}

DWORD CALLBACK resampler_proc(HSTREAM handle, void *buffer, DWORD length, void *user) {
	void* actual_buffer = buffer;
	size_t actual_length = length;

	BASS_SOX_RESAMPLER* resampler = (BASS_SOX_RESAMPLER*)user;

	//If there is an output_length then we're still processing some resampled content.
	if (resampler->output_length) {
		//Determine the remaining data length.
		DWORD output_remaining = resampler->output_length - resampler->output_position;
		if (output_remaining) {
			//If there is more remaining data than can be written then write what we can and increment the position.
			if (output_remaining > length) {
				memcpy(buffer, offset_output_buffer(resampler), length);
				resampler->output_position += length;
				return length;
			}
			//Looks like we have enough room to complete the current resampled content.
			else {
				memcpy(buffer, offset_output_buffer(resampler), output_remaining);
				//We adjust the length and output buffer to "hide" the data we just wrote.
				//It would be simpler to just return here but some drivers don't like the 
				//buffer partially populated so we must get some more data in other to
				//fill it.
				actual_length = length - output_remaining;
				actual_buffer = offset_buffer(resampler, output_remaining, actual_buffer);
			}
		}
	}

	if (!populate_resampler(resampler)) {
		//Failed to resample data for some reason.
		return resampler->bass_error;
	}

	//Looks like there's too much data so write what we can and set the output position.
	//This will be picked up by the next invocation of this routine.
	if (resampler->output_length > actual_length) {
		memcpy(actual_buffer, resampler->output_buffer, actual_length);
		resampler->output_position = actual_length;
		return length;
	}
	//There is enough output buffer space available so write everything and make 
	//sure the output position is empty.
	//This never happens by the way.
	else {
		memcpy(actual_buffer, resampler->output_buffer, resampler->output_length);
		resampler->output_position = 0;
		return resampler->output_length;
	}
}