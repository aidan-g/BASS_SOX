#include "bass/bass.h"

#ifndef BASSSOXASIODEF
#define BASSSOXASIODEF(f) WINAPI f
#endif

__declspec(dllexport)
BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Init)();

__declspec(dllexport)
BOOL BASSSOXASIODEF(BASS_SOX_ASIO_Free)();

__declspec(dllexport)
BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamGet)(DWORD* handle);

__declspec(dllexport)
BOOL BASSSOXASIODEF(BASS_SOX_ASIO_StreamSet)(DWORD handle);

__declspec(dllexport)
BOOL BASSSOXASIODEF(BASS_SOX_ASIO_ChannelEnable)(BOOL input, DWORD channel, void *user);

DWORD CALLBACK asio_sox_stream_proc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user);