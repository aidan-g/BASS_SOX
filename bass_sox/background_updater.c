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
	if (!get_resamplers(&resamplers, &length)) {
		return FALSE;
	}
	background_update_active = TRUE;
	while (!background_update_shutdown) {
		BOOL success = FALSE;
		for (a = 0; a < length; a++) {
			if (!resamplers[a]) {
				continue;
			}
			if (!resamplers[a]->background) {
				continue;
			}
			populate_resampler(resamplers[a]);
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

BOOL begin_background_update() {
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

BOOL end_background_update() {
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

BOOL ensure_background_update() {
	if (background_update_handle && !background_update_active) {
		end_background_update();
	}
	return begin_background_update();
}