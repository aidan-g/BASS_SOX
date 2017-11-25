#include "bass_sox.h"
#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"

//TODO: Make these configurable.
#define SOXR_QUALITY SOXR_VHQ | SOXR_LINEAR_PHASE
#define SOXR_THREADS 1

//Determine whether the specified flags imply BASS_SAMPLE_FLOAT.
BOOL is_float(DWORD flags) {
	return  (flags & BASS_SAMPLE_FLOAT) == BASS_SAMPLE_FLOAT;
}

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

//Construct a soxr_t for the specified resampler configuration.
BOOL create_soxr_resampler(BASS_SOX_RESAMPLER* resampler) {
	soxr_io_spec_t io_spec = soxr_io_spec(
		soxr_datatype(resampler->input_sample_size),
		soxr_datatype(resampler->output_sample_size));
	soxr_quality_spec_t quality_spec = soxr_quality_spec(SOXR_QUALITY, 0);
	soxr_runtime_spec_t runtime_spec = soxr_runtime_spec(SOXR_THREADS);

	return resampler->soxr = soxr_create(
		resampler->input_rate,
		resampler->output_rate,
		resampler->channels,
		&resampler->soxr_error,
		&io_spec,
		&quality_spec,
		&runtime_spec);
}

//Create a BASS stream containing a resampler payload for the specified frequency (freq).
HSTREAM BASSSOXDEF(BASS_SOX_StreamCreate)(DWORD freq, DWORD flags, DWORD handle, void *user) {

	BASS_CHANNELINFO input_channel_info;
	BASS_SOX_RESAMPLER* resampler;
	HSTREAM output_channel;
	BASS_CHANNELINFO output_channel_info;

	if (!BASS_ChannelGetInfo(handle, &input_channel_info)) {
		return BASS_ERROR_UNKNOWN;
	}

	resampler = create_resampler();
	output_channel = BASS_StreamCreate(freq, input_channel_info.chans, flags, &resampler_proc, resampler);
	if (output_channel == -1) {
		return BASS_ERROR_UNKNOWN;
	}

	if (!BASS_ChannelGetInfo(output_channel, &output_channel_info)) {
		return BASS_ERROR_UNKNOWN;
	}

	resampler->input_rate = input_channel_info.freq;
	resampler->output_rate = freq;
	resampler->channels = input_channel_info.chans;
	resampler->source_channel = handle;
	resampler->output_channel = output_channel;
	resampler->ratio = (resampler->output_rate / resampler->input_rate) + 1;
	resampler->input_sample_size = is_float(input_channel_info.flags) ? sizeof(float) : sizeof(short);
	resampler->input_frame_size = resampler->input_sample_size * resampler->channels;
	resampler->output_sample_size = is_float(output_channel_info.flags) ? sizeof(float) : sizeof(short);
	resampler->output_frame_size = resampler->output_sample_size * resampler->channels;

	if (!create_soxr_resampler(resampler)) {
		return BASS_SOX_ERROR_UNKNOWN;
	}

	register_resampler(resampler);
	if (!alloc_resampler_buffers(resampler)) {
		release_resampler(output_channel);
		return BASS_ERROR_NOTAVAIL;
	}

	return output_channel;
}

//Release the BASS stream and associated resampler resources.
BOOL BASSSOXDEF(BASS_SOX_StreamFree)(HSTREAM handle) {
	if (!BASS_StreamFree(handle)) {
		return BASS_ERROR_UNKNOWN;
	}
	return release_resampler(handle);
}

//Get the last error encountered by sox.
const char* BASSSOXDEF(BASS_SOX_GetLastError)(HSTREAM handle) {
	BASS_SOX_RESAMPLER resampler;
	if (!get_resampler(handle, &resampler)) {
		return "No such resampler.";
	}
	return resampler.soxr_error;
}