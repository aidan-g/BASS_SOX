#include "bass_sox.h"
#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"
#include "background_updater.h"
#include "resampler_lock.h"
#include "resampler_settings.h"

#define BUFFER_CLEAR_TIMEOUT 5000

BOOL is_initialized = FALSE;

//Determine whether the specified flags imply BASS_SAMPLE_FLOAT.
BOOL is_float(DWORD flags) {
	return  (flags & BASS_SAMPLE_FLOAT) == BASS_SAMPLE_FLOAT;
}

//Initialize.
BOOL BASSSOXDEF(BASS_SOX_Init)() {
	if (is_initialized) {
		return FALSE;
	}
	is_initialized = TRUE;
	return TRUE;
}

//Free.
BOOL BASSSOXDEF(BASS_SOX_Free)() {
	BOOL success = TRUE;
	if (!is_initialized) {
		success = FALSE;
	}
	else {
		DWORD a;
		DWORD length;
		BASS_SOX_RESAMPLER** resamplers = malloc(sizeof(BASS_SOX_RESAMPLER*) * MAX_RESAMPLERS);
		success &= background_update_end();
		if (resampler_registry_get_all(resamplers, &length)) {
			for (a = 0; a < length; a++) {
				if (!BASS_SOX_StreamFree(resamplers[a]->output_channel)) {
					success = FALSE;
				}
			}
		}
		free(resamplers);
	}
	is_initialized = FALSE;
	return success;
}

//Create a BASS stream containing a resampler payload for the specified frequency (freq).
HSTREAM BASSSOXDEF(BASS_SOX_StreamCreate)(DWORD freq, DWORD flags, DWORD handle, void *user) {
	BASS_CHANNELINFO input_channel_info;
	BASS_CHANNELINFO output_channel_info;
	BASS_SOX_RESAMPLER* resampler;
	HSTREAM output_channel;

	if (!BASS_ChannelGetInfo(handle, &input_channel_info)) {
		return 0;
	}

	if (input_channel_info.freq == freq) {
		//Input and output frequency are the same, nothing to do.
		return handle;
	}

	resampler = resampler_create();
	output_channel = BASS_StreamCreate(freq, input_channel_info.chans, flags, &resampler_proc, resampler);
	if (output_channel == 0) {
		return 0;
	}

	if (!BASS_ChannelGetInfo(output_channel, &output_channel_info)) {
		return 0;
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

BOOL BASSSOXDEF(BASS_SOX_StreamBufferClear)(DWORD handle) {
	BOOL success;
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	if (!resampler_lock_enter(resampler, BUFFER_CLEAR_TIMEOUT)) {
		return FALSE;
	}
	success = resampler_buffer_clear(resampler);
	if (!resampler_lock_exit(resampler)) {
		return FALSE;
	}
	return TRUE;
}

BOOL BASSSOXDEF(BASS_SOX_StreamBufferLength)(DWORD handle, DWORD* value) {
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	return resampler_buffer_length(resampler, value);
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
		break;
	case PHASE:
		settings->phase = value;
		break;
	case STEEP_FILTER:
		settings->steep_filter = value;
		break;
	case ALLOW_ALIASING:
		settings->allow_aliasing = value;
		break;
	case BUFFER_LENGTH:
		settings->buffer_length = value;
		break;
	case THREADS:
		settings->threads = value;
		break;
	case BACKGROUND:
		settings->background = value;
		if (settings->background) {
			background_update_begin();
		}
		break;
	case KEEP_ALIVE:
		settings->keep_alive = value;
		break;
	default:
		return FALSE;
	}
	//TODO: Does this need to be synchronized?
	resampler->reload = TRUE;
	return TRUE;
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
		break;
	case PHASE:
		*value = settings->phase;
		break;
	case STEEP_FILTER:
		*value = settings->steep_filter;
		break;
	case ALLOW_ALIASING:
		*value = settings->allow_aliasing;
		break;
	case BUFFER_LENGTH:
		*value = settings->buffer_length;
		break;
	case THREADS:
		*value = settings->threads;
		break;
	case BACKGROUND:
		*value = settings->background;
		break;
	case KEEP_ALIVE:
		*value = settings->keep_alive;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//Release the BASS stream and associated resampler resources.
BOOL BASSSOXDEF(BASS_SOX_StreamFree)(HSTREAM handle) {
	BOOL success = TRUE;
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	success &= BASS_StreamFree(handle);
	success &= resampler_registry_remove(resampler);
	success &= resampler_free(resampler);
	return success;
}

//Get the last error encountered by sox.
const char* BASSSOXDEF(BASS_SOX_GetLastError)(HSTREAM handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return "No such resampler.";
	}
	return resampler->soxr_error;
}