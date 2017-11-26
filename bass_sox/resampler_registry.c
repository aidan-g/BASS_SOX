#include "resampler_registry.h"

//10 should be enough for anybody.
#define MAX_RESAMPLERS 10

BASS_SOX_RESAMPLER* resamplers[MAX_RESAMPLERS];

//Create a new resampler.
BASS_SOX_RESAMPLER* create_resampler() {
	BASS_SOX_RESAMPLER* resampler = 0;
	if (!(resampler = calloc(sizeof(BASS_SOX_RESAMPLER), 1))) {
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
void release_soxr(BASS_SOX_RESAMPLER* resampler) {
	soxr_clear(resampler->soxr);
	soxr_delete(resampler->soxr);
	resampler->soxr = NULL;
}

void release_resampler_buffers(BASS_SOX_RESAMPLER* resampler) {
	if (resampler->input_buffer) {
		free(resampler->input_buffer);
	}
	if (resampler->output_buffer) {
		free(resampler->output_buffer);
	}
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
		release_soxr(resamplers[a]);
		release_resampler_buffers(resamplers[a]);
		free(resamplers[a]);
		resamplers[a] = NULL;
		return TRUE;
	}
	return FALSE;
}