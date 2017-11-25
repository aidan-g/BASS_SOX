#include "bass/bass.h"

#ifndef BASSSOXDEF
#define BASSSOXDEF(f) WINAPI f
#endif

//Some error happens.
#define BASS_SOX_ERROR_UNKNOWN	-1

//Create a BASS stream containing a resampler payload for the specified frequency (freq).
HSTREAM BASSSOXDEF(BASS_SOX_StreamCreate)(DWORD freq, DWORD flags, DWORD handle, void *user);

//Release the BASS stream and associated resampler resources.
BOOL BASSSOXDEF(BASS_SOX_StreamFree)(HSTREAM handle);

//Get the last error encountered by sox.
const char* BASSSOXDEF(BASS_SOX_GetLastError(HSTREAM handle));