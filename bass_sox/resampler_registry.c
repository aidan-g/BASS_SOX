#include "resampler_registry.h"
#include "resampler_lock.h"
#include "resampler_settings.h"

static BASS_SOX_RESAMPLER* resamplers[MAX_RESAMPLERS];

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

BOOL resampler_registry_get_all(BASS_SOX_RESAMPLER** __resamplers, DWORD* length)
{
	BYTE a;
	*length = 0;
	for (a = 0; a < MAX_RESAMPLERS; a++) {
		if (!resamplers[a]) {
			continue;
		}
		__resamplers[(*length)++] = resamplers[a];
	}
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