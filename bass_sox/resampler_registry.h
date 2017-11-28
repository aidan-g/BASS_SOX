#include "resampler.h"

BASS_SOX_RESAMPLER* create_resampler();

BOOL register_resampler(BASS_SOX_RESAMPLER* resampler);

BOOL get_resamplers(BASS_SOX_RESAMPLER*** resamplers, DWORD* length);

BOOL get_resampler(DWORD handle, BASS_SOX_RESAMPLER** resampler);

BOOL release_resampler(DWORD handle);

BOOL release_soxr(BASS_SOX_RESAMPLER* resampler);

BOOL release_resampler_buffers(BASS_SOX_RESAMPLER* resampler);