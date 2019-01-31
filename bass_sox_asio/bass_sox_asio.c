#ifdef _DEBUG
#include <stdio.h>
#endif

#include "bass_sox_asio.h"
#include "../bass_sox/resampler.h"
#include "../bass_sox/resampler_registry.h"
#include "bass/bassasio.h"

BOOL is_initialized = FALSE;
DWORD channel_handle = 0;

//I have no idea how to prevent linking against this routine in msvcrt.
//It doesn't exist on Windows XP.
//Hopefully it doesn't do anything important.
int _except_handler4_common() {
	return 0;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Init)() {
	if (is_initialized) {
		return FALSE;
	}
	channel_handle = 0;
	is_initialized = TRUE;
#if _DEBUG
	printf("BASS SOX ASIO initialized.\n");
#endif
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Free)() {
	if (!is_initialized) {
		return FALSE;
	}
	channel_handle = 0;
	is_initialized = FALSE;
#if _DEBUG
	printf("BASS SOX ASIO released.\n");
#endif
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamGet)(DWORD* handle) {
	if (!channel_handle) {
		return FALSE;
	}
	*handle = channel_handle;
#if _DEBUG
	printf("BASS SOX ASIO stream: %d.\n", channel_handle);
#endif
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamSet)(DWORD handle) {
	BASS_SOX_RESAMPLER* resampler;
	if (!resampler_registry_get(handle, &resampler)) {
#if _DEBUG
		printf("No resampler for channel: %d\n", handle);
#endif
		return FALSE;
	}
	channel_handle = handle;
#if _DEBUG
	printf("BASS SOX ASIO stream: %d.\n", channel_handle);
#endif
	return TRUE;
}

BOOL BASSSOXASIODEF(BASS_SOX_ASIO_ChannelEnable)(BOOL input, DWORD channel, void *user) {
	BOOL success = BASS_ASIO_ChannelEnable(input, channel, &asio_sox_stream_proc, user);
	if (!success) {
#if _DEBUG
		printf("BASS SOX ASIO enabled.\n");
#endif
	}
	return success;
}

DWORD CALLBACK asio_sox_stream_proc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user) {
	DWORD result;
	if (!channel_handle) {
		result = 0;
	}
	else {
		result = BASS_ChannelGetData(channel_handle, buffer, length);
		switch (result)
		{
		case BASS_STREAMPROC_END:
		case BASS_ERROR_UNKNOWN:
			result = 0;
			break;
		default:
#if _DEBUG
			printf("Write %d bytes to ASIO buffer\n", result);
#endif
			break;
		}
	}
	if (result < length) {
#if _DEBUG
		printf("Buffer underrun while writing to ASIO buffer.\n");
#endif
	}
	return result;
}