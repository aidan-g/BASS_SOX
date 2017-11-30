#include "resampler_settings.h"

BOOL resampler_settings_create(BASS_SOX_RESAMPLER* resampler) {
	resampler->settings = calloc(sizeof(BASS_SOX_RESAMPLER_SETTINGS), 1);
	if (!resampler->settings) {
		return FALSE;
	}
	resampler->settings->send_bass_streamproc_end = TRUE;
	return TRUE;
}

BOOL resampler_settings_free(BASS_SOX_RESAMPLER* resampler) {
	free(resampler->settings);
	return TRUE;
}