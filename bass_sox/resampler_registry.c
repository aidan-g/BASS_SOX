#include "resampler_registry.h"
#include "resampler_lock.h"
#include "resampler_settings.h"

//10 should be enough for anybody.
#define MAX_RESAMPLERS 10

BASS_SOX_RESAMPLER* resamplers[MAX_RESAMPLERS];

//Register a resampler.
BOOL resampler_registry_add(BASS_SOX_RESAMPLER* resampler) {
	BYTE a;
	for (a = 0; a < MAX_RESAMPLERS; a++) {
		if (resamplers[a]) {
			continue;
		}
		resamplers[a] = resampler;
		return TRUE;
	}
	return FALSE;
}

BOOL resampler_registry_remove(BASS_SOX_RESAMPLER* resampler) {
	BYTE a;
	for (a = 0; a < MAX_RESAMPLERS; a++) {
		if (!resamplers[a]) {
			continue;
		}
		if (resamplers[a] != resampler) {
			continue;
		}
		resamplers[a] = NULL;
		return TRUE;
	}
	return FALSE;
}

BOOL resampler_registry_get_all(BASS_SOX_RESAMPLER*** __resamplers, DWORD* length)
{
	*length = MAX_RESAMPLERS;
	*__resamplers = resamplers;
	return TRUE;
}

//Get the resampler for the specified BASS channel handle.
BOOL resampler_registry_get(DWORD handle, BASS_SOX_RESAMPLER** resampler) {
	BYTE a;
	for (a = 0; a < MAX_RESAMPLERS; a++) {
		if (!resamplers[a]) {
			continue;
		}
		if (resamplers[a]->output_channel != handle) {
			continue;
		}
		*resampler = resamplers[a];
		return TRUE;
	}
	return FALSE;
}