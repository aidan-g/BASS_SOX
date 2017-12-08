#include "bass_sox_asio.h"
#include "../bass_sox/resampler.h"
#include "../bass_sox/resampler_registry.h"
#include "bass/bassasio.h"

BOOL is_initialized = FALSE;
DWORD channel_handle = 0;

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Init)() {
	if (is_initialized) {
		return FALSE;
	}
	channel_handle = 0;
	is_initialized = TRUE;
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Free)() {
	if (!is_initialized) {
		return FALSE;
	}
	channel_handle = 0;
	is_initialized = FALSE;
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamGet)(DWORD* handle) {
	if (!channel_handle) {
		return FALSE;
	}
	*handle = channel_handle;
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamSet)(DWORD handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
		return FALSE;
	}
	channel_handle = handle;
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_ChannelEnable)(BOOL input, DWORD channel, void *user) {
	return BASS_ASIO_ChannelEnable(input, channel, &asio_sox_stream_proc, user);
}

DWORD CALLBACK asio_sox_stream_proc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user) {
	DWORD result;
	BASS_SOX_RESAMPLER * resampler;
	if (!channel_handle) {
		return 0;
	}
	if (!resampler_registry_get(channel_handle, &resampler)) {
		return 0;
	}
	result = resampler_proc(channel_handle, buffer, length, resampler);
	switch (result)
	{
	case BASS_STREAMPROC_END:
	case BASS_ERROR_UNKNOWN:
		result = 0;
	}
	return result;
}