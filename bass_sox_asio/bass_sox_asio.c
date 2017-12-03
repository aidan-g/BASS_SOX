#include "bass_sox_asio.h"
#include "../bass_sox/resampler.h"
#include "../bass_sox/resampler_registry.h"
#include "bass/bassasio.h"

BOOL is_initialized = FALSE;
BASS_SOX_RESAMPLER* resampler = NULL;

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Init)() {
	if (is_initialized) {
		return FALSE;
	}
	resampler = NULL;
	is_initialized = TRUE;
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Free)() {
	if (!is_initialized) {
		return FALSE;
	}
	resampler = NULL;
	is_initialized = FALSE;
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamGet)(DWORD* handle) {
	if (!resampler) {
		return FALSE;
	}
	*handle = resampler->output_channel;
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamSet)(DWORD handle) {
	if (!resampler_registry_get(handle, &resampler)) {
		resampler = NULL;
		return FALSE;
	}
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_ChannelEnable)(BOOL input, DWORD channel, void *user) {
	return BASS_ASIO_ChannelEnable(input, channel, &asio_sox_stream_proc, user);
}

DWORD CALLBACK asio_sox_stream_proc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user) {
	if (!resampler) {
		return 0;
	}
	DWORD result = resampler_proc(0, buffer, length, resampler);
	if (result == BASS_STREAMPROC_END) {
		result = 0;
	}
	return result;
}