#include "bass/bass.h"
#include "soxr/soxr.h"

#ifndef BASSSOXDEF
#define BASSSOXDEF(f) WINAPI f
#endif

//Some error happens.
#define BASS_SOX_ERROR_UNKNOWN	-1

typedef enum {
	QUALITY = 0,
	PHASE = 1,
	STEEP_FILTER = 2,
	ALLOW_ALIASING = 3
} BASS_SOX_ATTRIBUTE;

typedef enum {
	Q_QQ = SOXR_QQ,
	Q_LQ = SOXR_LQ,
	Q_MQ = SOXR_MQ,
	Q_HQ = SOXR_HQ,
	Q_VHQ = SOXR_VHQ,
	Q_16_BITQ = SOXR_16_BITQ,
	Q_20_BITQ = SOXR_20_BITQ,
	Q_24_BITQ = SOXR_24_BITQ,
	Q_28_BITQ = SOXR_28_BITQ,
	Q_32_BITQ = SOXR_32_BITQ
} BASS_SOX_QUALITY;

typedef enum {
	LINEAR = SOXR_LINEAR_PHASE,
	INTERMEDIATE = SOXR_INTERMEDIATE_PHASE,
	MINIMUM = SOXR_MINIMUM_PHASE
} BASS_SOX_PHASE;

//Create a BASS stream containing a resampler payload for the specified frequency (freq).
__declspec(dllexport)
HSTREAM BASSSOXDEF(BASS_SOX_StreamCreate)(DWORD freq, DWORD flags, DWORD handle, void *user);

//Set an attribute on the associated resampler.
__declspec(dllexport)
BOOL BASSSOXDEF(BASS_SOX_ChannelSetAttribute)(DWORD handle, DWORD attrib, DWORD value);

//Get an attribute from the associated resampler.
__declspec(dllexport)
BOOL BASSSOXDEF(BASS_SOX_ChannelGetAttribute)(DWORD handle, DWORD attrib, DWORD *value);

//Release the BASS stream and associated resampler resources.
__declspec(dllexport)
BOOL BASSSOXDEF(BASS_SOX_StreamFree)(HSTREAM handle);

//Get the last error encountered by sox.
__declspec(dllexport)
const char* BASSSOXDEF(BASS_SOX_GetLastError(HSTREAM handle));