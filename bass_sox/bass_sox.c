#include "bass_sox.h"
#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"
#include "background_updater.h"

//Determine whether the specified flags imply BASS_SAMPLE_FLOAT.
BOOL is_float(DWORD flags) {
	return  (flags & BASS_SAMPLE_FLOAT) == BASS_SAMPLE_FLOAT;
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

	//if (input_channel_info.freq == freq) {
	//	//Input and output frequency are the same, nothing to do.
	//	return handle;
	//}

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

	register_resampler(resampler);

	return output_channel;
}

BOOL BASSSOXDEF(BASS_SOX_StreamBuffer)(DWORD handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!get_resampler(handle, &resampler)) {
		return FALSE;
	}
	if (resampler->background) {
		return FALSE;
	}
	return populate_resampler(resampler);
}

BOOL BASSSOXDEF(BASS_SOX_ChannelSetAttribute)(DWORD handle, DWORD attrib, DWORD value) {
	BASS_SOX_RESAMPLER* resampler;
	if (!get_resampler(handle, &resampler)) {
		return FALSE;
	}
	switch (attrib) {
	case QUALITY:
		resampler->quality = value;
		return TRUE;
	case PHASE:
		resampler->phase = value;
		return TRUE;
	case STEEP_FILTER:
		resampler->steep_filter = value;
		return TRUE;
	case ALLOW_ALIASING:
		resampler->allow_aliasing = value;
		return TRUE;
	case BUFFER_LENGTH:
		resampler->buffer_length = value;
		return TRUE;
	case THREADS:
		resampler->threads = value;
		return TRUE;
	case BACKGROUND:
		resampler->background = value;
		if (resampler->background) {
			ensure_background_update();
		}
		return TRUE;
	}
	resampler->reload = TRUE;
	return FALSE;
}

BOOL BASSSOXDEF(BASS_SOX_ChannelGetAttribute)(DWORD handle, DWORD attrib, DWORD *value) {
	BASS_SOX_RESAMPLER* resampler;
	if (!get_resampler(handle, &resampler)) {
		return FALSE;
	}
	switch (attrib) {
	case QUALITY:
		*value = resampler->quality;
		return TRUE;
	case PHASE:
		*value = resampler->phase;
		return TRUE;
	case STEEP_FILTER:
		*value = resampler->steep_filter;
		return TRUE;
	case ALLOW_ALIASING:
		*value = resampler->allow_aliasing;
		return TRUE;
	case BUFFER_LENGTH:
		*value = resampler->buffer_length;
		return TRUE;
	case THREADS:
		*value = resampler->threads;
		return TRUE;
	case BACKGROUND:
		*value = resampler->background;
		return TRUE;
	}
	return FALSE;
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
	BASS_SOX_RESAMPLER* resampler;
	if (!get_resampler(handle, &resampler)) {
		return "No such resampler.";
	}
	return resampler->soxr_error;
}