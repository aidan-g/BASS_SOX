#include "resampler.h"

void* offset_buffer(BASS_SOX_RESAMPLER* resampler, DWORD position, void* buffer);

void* offset_output_buffer(BASS_SOX_RESAMPLER* resampler);

BOOL alloc_resampler_buffers(BASS_SOX_RESAMPLER* resampler);