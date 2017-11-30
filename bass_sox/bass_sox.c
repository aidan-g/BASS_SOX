#include "bass_sox.h"
#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"
#include "background_updater.h"
#include "resampler_lock.h"
#include "resampler_settings.h"

//Determine whether the specified flags imply BASS_SAMPLE_FLOAT.
BOOL is_float(DWORD flags) {
	return  (flags & BASS_SAMPLE_FLOAT) == BASS_SAMPLE_FLOAT;
}

//Initialize.
BOOL BASSSOXDEF(BASS_SOX_Init)() {
	//Nothing to do (yet).
	return TRUE;
}

//Free.
BOOL BASSSOXDEF(BASS_SOX_Free)() {
	DWORD a;
	DWORD length;
	BASS_SOX_RESAMPLER** resamplers;
	background_update_end();
	if (!resampler_registry_get_all(&resamplers, &length)) {
		return FALSE;
	}
	for (a = 0; a < length; a++) {
		if (!resamplers[a]) {
			continue;
		}
		if (!BASS_SOX_StreamFree(resamplers[a]->source_channel)) {
			return FALSE;
		}
	}
	return TRUE;
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

	if (input_channel_info.freq == freq) {
		//Input and output frequency are the same, nothing to do.
		return handle;
	}

	resampler = resampler_create();
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
	resampler->input_sample_size = is_float(input_channel_info.flags) ? sizeof(float) : sizeof(short);
	resampler->input_frame_size = resampler->input_sample_size * resampler->channels;
	resampler->output_sample_size = is_float(output_channel_info.flags) ? sizeof(float) : sizeof(short);
	resampler->output_frame_size = resampler->output_sample_size * resampler->channels;

	resampler_settings_create(resampler);
	resampler_lock_create(resampler);
	resampler_registry_add(resampler);

	return output_channel;
}

BOOL BASSSOXDEF(BASS_SOX_StreamBuffer)(DWORD handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	return resampler_populate(resampler);
}

BOOL BASSSOXDEF(BASS_SOX_ChannelSetAttribute)(DWORD handle, DWORD attrib, DWORD value) {
	BASS_SOX_RESAMPLER* resampler;
	BASS_SOX_RESAMPLER_SETTINGS* settings;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	settings = resampler->settings;
	switch (attrib) {
	case QUALITY:
		settings->quality = value;
		return TRUE;
	case PHASE:
		settings->phase = value;
		return TRUE;
	case STEEP_FILTER:
		settings->steep_filter = value;
		return TRUE;
	case ALLOW_ALIASING:
		settings->allow_aliasing = value;
		return TRUE;
	case BUFFER_LENGTH:
		settings->buffer_length = value;
		return TRUE;
	case THREADS:
		settings->threads = value;
		return TRUE;
	case BACKGROUND:
		settings->background = value;
		if (settings->background) {
			background_update_begin();
		}
		return TRUE;
	case SEND_BASS_STREAMPROC_END:
		settings->send_bass_streamproc_end = value;
		return TRUE;
	}
	resampler->reload = TRUE;
	return FALSE;
}

BOOL BASSSOXDEF(BASS_SOX_ChannelGetAttribute)(DWORD handle, DWORD attrib, DWORD *value) {
	BASS_SOX_RESAMPLER* resampler;
	BASS_SOX_RESAMPLER_SETTINGS* settings;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	settings = resampler->settings;
	switch (attrib) {
	case QUALITY:
		*value = settings->quality;
		return TRUE;
	case PHASE:
		*value = settings->phase;
		return TRUE;
	case STEEP_FILTER:
		*value = settings->steep_filter;
		return TRUE;
	case ALLOW_ALIASING:
		*value = settings->allow_aliasing;
		return TRUE;
	case BUFFER_LENGTH:
		*value = settings->buffer_length;
		return TRUE;
	case THREADS:
		*value = settings->threads;
		return TRUE;
	case BACKGROUND:
		*value = settings->background;
		return TRUE;
	case SEND_BASS_STREAMPROC_END:
		*value = settings->send_bass_streamproc_end;
		return TRUE;
	}
	return FALSE;
}

//Release the BASS stream and associated resampler resources.
BOOL BASSSOXDEF(BASS_SOX_StreamFree)(HSTREAM handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	resampler_registry_remove(resampler);
	resampler_free(resampler);
	if (!BASS_StreamFree(handle)) {
		return BASS_ERROR_UNKNOWN;
	}
	return TRUE;
}

//Get the last error encountered by sox.
const char* BASSSOXDEF(BASS_SOX_GetLastError)(HSTREAM handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return "No such resampler.";
	}
	return resampler->soxr_error;
}