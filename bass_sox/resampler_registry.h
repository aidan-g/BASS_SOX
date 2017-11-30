#include "resampler.h"

BOOL resampler_registry_add(BASS_SOX_RESAMPLER* resampler);

BOOL resampler_registry_remove(BASS_SOX_RESAMPLER* resampler);

BOOL resampler_registry_get_all(BASS_SOX_RESAMPLER*** resamplers, DWORD* length);

BOOL resampler_registry_get(DWORD handle, BASS_SOX_RESAMPLER** resampler);