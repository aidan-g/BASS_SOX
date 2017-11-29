#include "resampler_registry.h"
#include "resampler_lock.h"

//10 should be enough for anybody.
#define MAX_RESAMPLERS 10

BASS_SOX_RESAMPLER* resamplers[MAX_RESAMPLERS];

//Create a new resampler.
BASS_SOX_RESAMPLER* create_resampler() {
	BASS_SOX_RESAMPLER* resampler = calloc(sizeof(BASS_SOX_RESAMPLER), 1);
	resampler->settings = calloc(sizeof(BASS_SOX_RESAMPLER_SETTINGS), 1);
	return resampler;
}

//Register a resampler.
BOOL register_resampler(BASS_SOX_RESAMPLER* resampler) {
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

BOOL get_resamplers(BASS_SOX_RESAMPLER*** __resamplers, DWORD* length)
{
	*length = MAX_RESAMPLERS;
	*__resamplers = resamplers;
	return TRUE;
}

//Get the resampler for the specified BASS channel handle.
BOOL get_resampler(DWORD handle, BASS_SOX_RESAMPLER** resampler) {
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

//Release the soxr resampler.
BOOL release_soxr(BASS_SOX_RESAMPLER* resampler) {
	soxr_clear(resampler->soxr);
	soxr_delete(resampler->soxr);
	resampler->soxr = NULL;
	return TRUE;
}

//Release the resampler and related resources for the specified BASS channel handle.
BOOL release_resampler(DWORD handle) {
	BYTE a;
	for (a = 0; a < MAX_RESAMPLERS; a++) {
		if (!resamplers[a]) {
			continue;
		}
		if (resamplers[a]->output_channel != handle) {
			continue;
		}
		if (!release_soxr(resamplers[a])) {
			return FALSE;
		}
		if (!release_resampler_buffers(resamplers[a])) {
			return FALSE;
		}
		if (!resampler_lock_free(resamplers[a])) {
			return FALSE;
		}
		free(resamplers[a]);
		resamplers[a] = NULL;
		return TRUE;
	}
	return FALSE;
}