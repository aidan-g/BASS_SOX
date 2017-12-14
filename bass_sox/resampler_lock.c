#ifdef _DEBUG
#include <stdio.h>
#endif

#include "resampler_lock.h"

BOOL resampler_lock_create(BASS_SOX_RESAMPLER* resampler) {
	if (resampler->lock) {
		resampler_lock_free(resampler);
	}
	resampler->lock = CreateSemaphore(NULL, 1, 1, NULL);
	if (!resampler->lock) {
#if _DEBUG
		printf("Failed to create semaphore.\n");
#endif
		return FALSE;
	}
	return TRUE;
}

BOOL resampler_lock_free(BASS_SOX_RESAMPLER* resampler) {
	BOOL success;
	if (!resampler->lock) {
		return TRUE;
	}
	success = CloseHandle(resampler->lock);
	if (!success) {
#if _DEBUG
		printf("Failed to release semaphore.\n");
#endif
	}
	else {
		resampler->lock = NULL;
	}
	return success;
}

BOOL resampler_lock_enter(BASS_SOX_RESAMPLER* resampler, DWORD timeout) {
	DWORD result = WaitForSingleObject(resampler->lock, timeout);
	if (result == WAIT_OBJECT_0) {
		return TRUE;
	}
#if _DEBUG
	printf("Failed to enter semaphore.\n");
#endif
	return FALSE;
}

BOOL resampler_lock_exit(BASS_SOX_RESAMPLER* resampler) {
	BOOL success = ReleaseSemaphore(resampler->lock, 1, NULL);
	if (!success) {
#if _DEBUG
		printf("Failed to exit semaphore.\n");
#endif
	}
	return success;
}