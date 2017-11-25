#include "resampler_buffer.h"

//Return a pointer to buffer offset by the specified position.
void* offset_buffer(BASS_SOX_RESAMPLER* resampler, DWORD position, void* buffer) {
	//We actually have the size of the data in resampler.sample_size 
	//but we need to cast in order to do pointer arithmetic.
	//Kind of awkward.
	if (position) {
		if (resampler->output_sample_size == sizeof(short)) {
			return (short*)buffer + (position / sizeof(short));
		}
		if (resampler->output_sample_size == sizeof(float)) {
			return (float*)buffer + (position / sizeof(float));
		}
	}
	return buffer;
}

//A convinient routine to get the output buffer offset by the output position.
void* offset_output_buffer(BASS_SOX_RESAMPLER* resampler) {
	return offset_buffer(resampler, resampler->output_position, resampler->output_buffer);
}

//Allocate the "optimal" resampler buffers.
BOOL alloc_resampler_buffers(BASS_SOX_RESAMPLER* resampler) {
	resampler->input_buffer_length = resampler->input_rate * resampler->input_frame_size;
	resampler->input_buffer = malloc(resampler->input_buffer_length);
	if (!resampler->input_buffer) {
		return FALSE;
	}
	resampler->output_buffer_length = resampler->output_rate * resampler->output_frame_size;
	resampler->output_buffer = malloc(resampler->output_buffer_length);
	if (!resampler->output_buffer) {
		return FALSE;
	}
	return TRUE;
}