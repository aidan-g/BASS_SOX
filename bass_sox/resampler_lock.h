#include "resampler.h"

BOOL resampler_lock_create(BASS_SOX_RESAMPLER* resampler);

BOOL resampler_lock_free(BASS_SOX_RESAMPLER* resampler);

BOOL resampler_lock_enter(BASS_SOX_RESAMPLER* resampler, DWORD timeout);

BOOL resampler_lock_exit(BASS_SOX_RESAMPLER* resampler);