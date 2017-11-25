#include "resampler_registry.h"

//10 should be enough for anybody.
#define MAX_RESAMPLERS 10

BASS_SOX_RESAMPLER* resamplers[MAX_RESAMPLERS];

//Create a new resampler.
BASS_SOX_RESAMPLER* create_resampler() {
	BASS_SOX_RESAMPLER* resampler = 0;
	if (!(resampler = calloc(sizeof(BASS_SOX_RESAMPLER*), 1))) {
		return NULL;
	}
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

//Get the resampler for the specified BASS channel handle.
BOOL get_resampler(DWORD handle, BASS_SOX_RESAMPLER* resampler) {
	BYTE a;
	for (a = 0; a < MAX_RESAMPLERS; a++) {
		if (!resamplers[a]) {
			continue;
		}
		if (resamplers[a]->output_channel != handle) {
			continue;
		}
		resampler = resamplers[a];
		return TRUE;
	}
	return FALSE;
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
		if (resamplers[a]->input_buffer) {
			free(resamplers[a]->input_buffer);
		}
		if (resamplers[a]->output_buffer) {
			free(resamplers[a]->output_buffer);
		}
		soxr_clear(resamplers[a]->soxr);
		soxr_delete(resamplers[a]->soxr);
		free(resamplers[a]);
		resamplers[a] = NULL;
		return TRUE;
	}
	return FALSE;
}