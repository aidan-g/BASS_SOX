#include <stdio.h>
#include "resampler.h"
#include "resampler_buffer.h"

DWORD CALLBACK resampler_proc(HSTREAM handle, void *buffer, DWORD length, void *user) {
	void* actual_buffer = buffer;
	size_t actual_length = length;

	BASS_SOX_RESAMPLER* resampler = (BASS_SOX_RESAMPLER*)user;
	DWORD source_channel_read_length;

	size_t input_frames;
	size_t output_frames;
	size_t resample_input_read_length;
	size_t resample_output_written_length;

	//If there is an output_position then we're still processing some resampled content.
	if (resampler->output_position) {
		//Determine the remaining data length.
		DWORD output_remaining = resampler->output_length - resampler->output_position;
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

	//Read the source data to be resampled, check for read errors.
	source_channel_read_length = BASS_ChannelGetData(
		resampler->source_channel,
		resampler->input_buffer,
		resampler->input_buffer_length);

	if (source_channel_read_length == BASS_ERR) {
#ifdef DEBUG
		printf("Failed to read from source channel: %d\n", BASS_ErrorGetCode());
#endif
		return 0;
	}

	if (source_channel_read_length == BASS_ERR_END) {
#ifdef DEBUG
		printf("Failed to read from source channel: Ended.\n");
#endif
		return BASS_STREAMPROC_END;
	}

	//Calculate how many frames can be read/written.
	input_frames = source_channel_read_length / resampler->input_frame_size;
	output_frames = resampler->output_buffer_length / resampler->output_frame_size;
	resample_input_read_length;
	resample_output_written_length;

	//Inboke soxr to resample the input data and check for errors.
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
		return 0;
	}

	if (resample_input_read_length < input_frames) {
#ifdef DEBUG
		printf("Input was not completed, buffer is too small?\n");
#endif
		return 0;
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