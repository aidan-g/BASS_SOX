#include <stdio.h>
#include <conio.h>
#include "bass/bass.h"
#include "../bass_sox/bass_sox.h"

int main()
{
	int output_rate = 192000;
	DWORD source_channel;
	DWORD playback_channel;
	QWORD channel_length;
	double channel_length_seconds;

	//Start BASS.
	if (!BASS_Init(-1, output_rate, 0, 0, NULL)) {
		printf("Failed to initialize BASS: %d\n", BASS_ErrorGetCode());
		return 1;
	}

	if (!BASS_SOX_Init()) {
		printf("Failed to initialize SOX.");
		return 1;
	}

	//Create a DECODE stream for a crappy song.
	source_channel = BASS_StreamCreateFile(FALSE, "D:\\Source\\Prototypes\\Resources\\1 - 6 - DYE (game version).mp3", 0, 0, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
	if (source_channel == 0) {
		printf("Failed to create source stream: %d\n", BASS_ErrorGetCode());
		return 1;
	}

	//Create the re sampler stream.
	playback_channel = BASS_SOX_StreamCreate(output_rate, BASS_SAMPLE_FLOAT, source_channel, NULL);
	if (playback_channel == 0) {
		printf("Failed to create playback stream: %d\n", BASS_ErrorGetCode());
		return 1;
	}

	BASS_SOX_ChannelSetAttribute(playback_channel, QUALITY, Q_VHQ);
	BASS_SOX_ChannelSetAttribute(playback_channel, PHASE, LINEAR);
	BASS_SOX_ChannelSetAttribute(playback_channel, STEEP_FILTER, TRUE);
	BASS_SOX_ChannelSetAttribute(playback_channel, ALLOW_ALIASING, TRUE);
	BASS_SOX_ChannelSetAttribute(playback_channel, BUFFER_LENGTH, 5);
	BASS_SOX_ChannelSetAttribute(playback_channel, THREADS, 2);
	BASS_SOX_ChannelSetAttribute(playback_channel, BACKGROUND, TRUE);

	if (!BASS_SOX_StreamBuffer(playback_channel)) {
		printf("Failed to buffer playback stream: %d\n", BASS_ErrorGetCode());
		return 1;
	}

	//Play the re sampler stream.
	if (!BASS_ChannelPlay(playback_channel, FALSE)) {
		printf("Failed to play stream: %d\n", BASS_ErrorGetCode());
		return 1;
	}

	//Calculate the source length.
	channel_length = BASS_ChannelGetLength(source_channel, BASS_POS_BYTE);
	channel_length_seconds = BASS_ChannelBytes2Seconds(source_channel, channel_length);

	do {
		QWORD channel_position;
		double channel_position_seconds;

		//Check the channel is active otherwise break.
		DWORD channel_active = BASS_ChannelIsActive(playback_channel);
		if (channel_active == BASS_ACTIVE_STOPPED) {
			break;
		}

		if (_kbhit()) {
			_getch();
			BASS_ChannelSetPosition(source_channel, BASS_ChannelSeconds2Bytes(source_channel, channel_length_seconds - 10), BASS_POS_BYTE);
			BASS_SOX_StreamBufferClear(playback_channel);
		}

		//Calculate the source position and write it out.
		channel_position = BASS_ChannelGetPosition(playback_channel, BASS_POS_BYTE);
		channel_position_seconds = BASS_ChannelBytes2Seconds(playback_channel, channel_position);
		printf("%d/%d\n", (int)channel_position_seconds, (int)channel_length_seconds);
		Sleep(1000);
	} while (TRUE);

	//Free resources, little pauses expose bugs.
	BASS_SOX_StreamFree(playback_channel);
	BASS_StreamFree(source_channel);
	BASS_SOX_Free();
	BASS_Free();

	return 0;
}

