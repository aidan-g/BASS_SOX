#include "resampler.h"
#include "resampler_registry.h"

#define UPDATE_INTERVAL 100

#define UPDATE_TIMEOUT 0

BOOL background_update_shutdown = FALSE;
volatile BOOL background_update_active = FALSE;
HANDLE background_update_handle = NULL;

DWORD WINAPI background_update(void* args) {
	DWORD a;
	DWORD length;
	BASS_SOX_RESAMPLER** resamplers;
	background_update_active = TRUE;
	while (!background_update_shutdown) {
		BOOL success = FALSE;
		if (!resampler_registry_get_all(&resamplers, &length)) {
			return FALSE;
		}
		for (a = 0; a < length; a++) {
			if (!resamplers[a]) {
				continue;
			}
			if (!resamplers[a]->settings->background) {
				continue;
			}
			resampler_populate(resamplers[a]);
			success = TRUE;
		}
		if (!success) {
			break;
		}
		Sleep(UPDATE_INTERVAL);
	}
	background_update_active = FALSE;
	return TRUE;
}

BOOL background_update_begin() {
	if (background_update_active) {
		return TRUE;
	}
	background_update_shutdown = FALSE;
	background_update_handle = CreateThread(NULL, 0, background_update, NULL, 0, NULL);
	if (!background_update_handle) {
		return FALSE;
	}
	return TRUE;
}

BOOL background_update_end() {
	DWORD result;
	if (!background_update_handle) {
		return TRUE;
	}
	background_update_shutdown = TRUE;
	result = WaitForSingleObject(background_update_handle, INFINITE);
	if (!CloseHandle(background_update_handle)) {
		return FALSE;
	}
	background_update_handle = NULL;
	if (result == WAIT_OBJECT_0) {
		return TRUE;
	}
	return FALSE;
}