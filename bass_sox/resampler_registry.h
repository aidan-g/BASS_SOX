#include "resampler.h"

//10 should be enough for anybody.
#define MAX_RESAMPLERS 10

BOOL resampler_registry_add(BASS_SOX_RESAMPLER* resampler);

BOOL resampler_registry_remove(BASS_SOX_RESAMPLER* resampler);

BOOL resampler_registry_get_all(BASS_SOX_RESAMPLER** resamplers, DWORD* length);

__declspec(dllexport)
BOOL resampler_registry_get(DWORD handle, BASS_SOX_RESAMPLER** resampler);