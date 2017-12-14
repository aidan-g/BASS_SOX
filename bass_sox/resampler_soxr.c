#ifdef _DEBUG
#include <stdio.h>
#endif

#include "resampler_soxr.h"

#define SOXR_DEFAULT_THREADS 1
#define SOXR_DEFAULT_QUALITY SOXR_VHQ
#define SOXR_DEFAULT_PHASE SOXR_LINEAR_PHASE

soxr_datatype_t soxr_datatype(size_t sample_size) {
	soxr_datatype_t result;
	if (sample_size == sizeof(short)) {
		result = SOXR_INT16_I;
	}
	else if (sample_size == sizeof(float)) {
		result = SOXR_FLOAT32_I;
	}
	else {
		result = 0;
	}
#if _DEBUG
	printf("Sox data type for sample size: %d = %d\n", sample_size, result);
#endif
	return result;
}

soxr_quality_spec_t get_quality_spec(BASS_SOX_RESAMPLER* resampler) {
	unsigned long recipe = 0;
	unsigned long flags = 0;
	BASS_SOX_RESAMPLER_SETTINGS* settings = resampler->settings;
	if (settings->quality) {
		recipe |= settings->quality;
	}
	else {
		recipe |= SOXR_DEFAULT_QUALITY;
	}
	if (settings->phase) {
		recipe |= settings->phase;
	}
	else {
		recipe |= SOXR_DEFAULT_PHASE;
	}
	if (settings->steep_filter) {
		recipe |= SOXR_STEEP_FILTER;
	}
	if (settings->allow_aliasing) {
		//No longer implemented.
	}
#if _DEBUG
	printf("Sox quality spec: %d = %d\n", recipe, flags);
#endif
	return soxr_quality_spec(recipe, flags);
}

soxr_runtime_spec_t get_runtime_spec(BASS_SOX_RESAMPLER* resampler) {
	unsigned int threads = 0;
	BASS_SOX_RESAMPLER_SETTINGS* settings = resampler->settings;
	if (settings->threads) {
		threads = settings->threads;
	}
	else {
		threads = SOXR_DEFAULT_THREADS;
	}
#if _DEBUG
	printf("Sox runtime spec: threads = %d\n", threads);
#endif
	return soxr_runtime_spec(threads);
}

BOOL resampler_soxr_create(BASS_SOX_RESAMPLER* resampler) {
	soxr_io_spec_t io_spec = soxr_io_spec(
		soxr_datatype(resampler->input_sample_size),
		soxr_datatype(resampler->output_sample_size));
	soxr_quality_spec_t quality_spec = get_quality_spec(resampler);
	soxr_runtime_spec_t runtime_spec = get_runtime_spec(resampler);

	if (resampler->soxr) {
		resampler_soxr_free(resampler);
	}

	resampler->soxr = soxr_create(
		resampler->input_rate,
		resampler->output_rate,
		resampler->channels,
		&resampler->soxr_error,
		&io_spec,
		&quality_spec,
		&runtime_spec);

	if (!resampler->soxr) {
#if _DEBUG
		printf("Failed to create soxr.\n");
#endif
		return FALSE;
	}

#if _DEBUG
	printf("Created soxr: %d.\n", resampler->soxr);
#endif
	return TRUE;
}

BOOL resampler_soxr_free(BASS_SOX_RESAMPLER* resampler) {
	if (!resampler->soxr) {
		return TRUE;
	}
#if _DEBUG
	printf("Releasing soxr: %d.\n", resampler->soxr);
#endif
	soxr_clear(resampler->soxr);
	soxr_delete(resampler->soxr);
	resampler->soxr = NULL;
	return TRUE;
}