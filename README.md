# BASS_SOX

A resampler plugin for BASS which uses the soxr library with .NET bindings.

bass.dll is required for native projects.
ManagedBass is required for .NET projects.

A simple example;

```c
#include <stdio.h>
#include "bass/bass.h"
#include "bass_sox/bass_sox.h"

int main()
{
	int output_rate = 192000;
	DWORD source_channel;
	DWORD playback_channel;

	if (!BASS_Init(-1, output_rate, 0, 0, NULL)) {
		return 1;
	}

	source_channel = BASS_StreamCreateFile(FALSE, "C:\\Source\\Prototypes\\Resources\\1 - 6 - DYE (game version).mp3", 0, 0, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
	if (source_channel == 0) {
		return 1;
	}

	playback_channel = BASS_SOX_StreamCreate(output_rate, BASS_SAMPLE_FLOAT, source_channel, NULL);
	if (playback_channel == 0) {
		return 1;
	}

	//These settings are optional.
	//Defaults to medium quality, 1 second buffer and 1 thread.
	BASS_SOX_ChannelSetAttribute(playback_channel, QUALITY, Q_VHQ);
	BASS_SOX_ChannelSetAttribute(playback_channel, PHASE, LINEAR);
	BASS_SOX_ChannelSetAttribute(playback_channel, STEEP_FILTER, TRUE);
	BASS_SOX_ChannelSetAttribute(playback_channel, ALLOW_ALIASING, TRUE);
	BASS_SOX_ChannelSetAttribute(playback_channel, BUFFER_LENGTH, 2);
	BASS_SOX_ChannelSetAttribute(playback_channel, THREADS, 2);

	if (!BASS_SOX_StreamBuffer(playback_channel)) {
		return 1;
	}

	if (!BASS_ChannelPlay(playback_channel, FALSE)) {
		return 1;
	}

	do {
		DWORD channel_active = BASS_ChannelIsActive(source_channel);
		if (channel_active == BASS_ACTIVE_STOPPED) {
			break;
		}
		Sleep(1000);
	} while (TRUE);

	BASS_SOX_StreamFree(playback_channel);
	BASS_StreamFree(source_channel);
	BASS_Free();

	return 0;
}
```
