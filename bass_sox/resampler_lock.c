#include "resampler_lock.h"

BOOL resampler_lock_create(BASS_SOX_RESAMPLER* resampler) {
	resampler_lock_free(resampler);
	resampler->lock = CreateSemaphore(NULL, 1, 1, NULL);
	if (!resampler->lock) {
		return FALSE;
	}
	return TRUE;
}

BOOL resampler_lock_free(BASS_SOX_RESAMPLER* resampler) {
	if (!resampler->lock) {
		return FALSE;
	}
	return CloseHandle(resampler->lock);
}

BOOL resampler_lock_enter(BASS_SOX_RESAMPLER* resampler, DWORD timeout) {
	DWORD result = WaitForSingleObject(resampler->lock, timeout);
	if (result == WAIT_OBJECT_0) {
		return TRUE;
	}
	return FALSE;
}

BOOL resampler_lock_exit(BASS_SOX_RESAMPLER* resampler) {
	return ReleaseSemaphore(resampler->lock, 1, NULL);
}