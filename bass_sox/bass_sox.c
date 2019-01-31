#ifdef _DEBUG
#include <stdio.h>
#endif

#include "bass_sox.h"
#include "resampler.h"
#include "resampler_buffer.h"
#include "resampler_registry.h"
#include "resampler_updater.h"
#include "resampler_lock.h"
#include "resampler_settings.h"

#define BUFFER_CLEAR_TIMEOUT 5000

BOOL is_initialized = FALSE;

//I have no idea how to prevent linking against this routine in msvcrt.
//It doesn't exist on Windows XP.
//Hopefully it doesn't do anything important.
int _except_handler4_common() {
	return 0;
}

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
#if _DEBUG
	printf("BASS SOX initialized.\n");
#endif
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
		static BASS_SOX_RESAMPLER* resamplers[MAX_RESAMPLERS];
		if (resampler_registry_get_all(resamplers, &length)) {
			for (a = 0; a < length; a++) {
				if (!BASS_SOX_StreamFree(resamplers[a]->output_channel)) {
					success = FALSE;
				}
			}
		}
	}
	if (success) {
		is_initialized = FALSE;
#if _DEBUG
		printf("BASS SOX released.\n");
#endif
	}
	else {
#if _DEBUG
		printf("Failed to release BASS SOX.\n");
#endif
	}
	return success;
}

//Create a BASS stream containing a resampler payload for the specified frequency (freq).
HSTREAM BASSSOXDEF(BASS_SOX_StreamCreate)(DWORD freq, DWORD flags, DWORD handle, void *user) {
	BOOL success = TRUE;
	BASS_CHANNELINFO input_channel_info;
	BASS_CHANNELINFO output_channel_info;
	BASS_SOX_RESAMPLER* resampler;
	HSTREAM output_channel;
	if (!is_initialized) {
		return FALSE;
	}

	if (!BASS_ChannelGetInfo(handle, &input_channel_info)) {
#if _DEBUG
		printf("Failed to get info for channel: %d\n", handle);
#endif
		return 0;
	}

	if (input_channel_info.freq == freq) {
#if _DEBUG
		printf("Channel does not require resampling.\n");
#endif
		return handle;
	}

	resampler = resampler_create();
	output_channel = BASS_StreamCreate(freq, input_channel_info.chans, flags, &resampler_proc, resampler);
	if (output_channel == 0) {
#if _DEBUG
		printf("Failed to create output channel.\n");
#endif
		return 0;
	}

	if (!BASS_ChannelGetInfo(output_channel, &output_channel_info)) {
#if _DEBUG
		printf("Failed to get info for channel: %d\n", output_channel);
#endif
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

	success &= resampler_settings_create(resampler);
	success &= resampler_lock_create(resampler);
	success &= resampler_registry_add(resampler);

	if (!success) {
#if _DEBUG
		printf("Failed to properly construct resampler, releasing it.\n");
#endif
		BASS_StreamFree(output_channel);
		resampler_registry_remove(resampler);
		resampler_free(resampler);
		return 0;
	}

#if _DEBUG
	printf("Created resampler channel: %d => %d\n", handle, output_channel);
#endif

	return output_channel;
}

BOOL BASSSOXDEF(BASS_SOX_StreamBuffer)(DWORD handle) {
	BOOL success = FALSE;
	BASS_SOX_RESAMPLER* resampler;
	if (!is_initialized) {
		return FALSE;
	}
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
		return FALSE;
	}
#if _DEBUG
	printf("Populating buffers.\n");
#endif
	while (resampler_populate(resampler)) {
		success = TRUE;
	}
	return success;
}

BOOL BASSSOXDEF(BASS_SOX_StreamBufferClear)(DWORD handle) {
	BOOL success;
	BASS_SOX_RESAMPLER* resampler;
	if (!is_initialized) {
		return FALSE;
	}
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
		return FALSE;
	}
	if (!resampler_lock_enter(resampler, BUFFER_CLEAR_TIMEOUT)) {
		return FALSE;
	}
#if _DEBUG
	printf("Clearing buffers.\n");
#endif
	success = resampler_buffer_clear(resampler);
	if (!resampler_lock_exit(resampler)) {
		return FALSE;
	}
	return success;
}

BOOL BASSSOXDEF(BASS_SOX_StreamBufferLength)(DWORD handle, DWORD* value) {
	BASS_SOX_RESAMPLER* resampler;
	if (!is_initialized) {
		return FALSE;
	}
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
		return FALSE;
	}
#if _DEBUG
	printf("Getting buffer length\n");
#endif
	return resampler_buffer_length(resampler, value);
}

BOOL BASSSOXDEF(BASS_SOX_ChannelSetAttribute)(DWORD handle, DWORD attrib, DWORD value) {
	BASS_SOX_RESAMPLER* resampler;
	BASS_SOX_RESAMPLER_SETTINGS* settings;
	if (!is_initialized) {
		return FALSE;
	}
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
		return FALSE;
	}
#if _DEBUG
	printf("Setting resampler attribute: %d = %d\n", attrib, value);
#endif
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
			resampler_updater_start(resampler);
		}
		else {
			resampler_updater_stop(resampler);
		}
		break;
	case KEEP_ALIVE:
		settings->keep_alive = value;
		break;
	default:
#if _DEBUG
		printf("Unrecognized resampler attribute: %d\n", attrib);
#endif
		return FALSE;
	}
	//TODO: Does this need to be synchronized?
#if _DEBUG
	printf("Requesting reload.\n");
#endif
	resampler->reload = TRUE;
	return TRUE;
}

BOOL BASSSOXDEF(BASS_SOX_ChannelGetAttribute)(DWORD handle, DWORD attrib, DWORD *value) {
	BASS_SOX_RESAMPLER* resampler;
	BASS_SOX_RESAMPLER_SETTINGS* settings;
	if (!is_initialized) {
		return FALSE;
	}
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
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
#if _DEBUG
		printf("Unrecognized resampler attribute: %d\n", attrib);
#endif
		return FALSE;
	}
#if _DEBUG
	printf("Getting resampler attribute: %d = %d\n", attrib, value);
#endif
	return TRUE;
}

//Release the BASS stream and associated resampler resources.
BOOL BASSSOXDEF(BASS_SOX_StreamFree)(HSTREAM handle) {
	BOOL success = TRUE;
	BASS_SOX_RESAMPLER* resampler;
	if (!is_initialized) {
		return FALSE;
	}
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
		return FALSE;
	}
#if _DEBUG
	printf("Releasing resampler.\n");
#endif
	success &= BASS_StreamFree(handle);
	success &= resampler_updater_stop(resampler);
	success &= resampler_registry_remove(resampler);
	success &= resampler_free(resampler);
	return success;
}

//Get the last error encountered by sox.
const char* BASSSOXDEF(BASS_SOX_GetLastError)(HSTREAM handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!is_initialized) {
		return FALSE;
	}
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
		return "No such resampler.";
	}
	return resampler->soxr_error;
}