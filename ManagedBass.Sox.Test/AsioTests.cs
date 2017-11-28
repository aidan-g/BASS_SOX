using ManagedBass.Asio;
using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ManagedBass.Sox.Test
{
    public class AsioTests
    {
        const int OUTPUT_RATE = 192000;

        [Test]
        public void Test001()
        {
            if (!Bass.Init(Bass.NoSoundDevice, OUTPUT_RATE))
            {
                Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var sourceChannel = Bass.CreateStream(@"C:\Source\Prototypes\Resources\1 - 6 - DYE (game version).mp3", 0, 0, BassFlags.Decode | BassFlags.Float);
            if (sourceChannel == -1)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var playbackChannel = BassSox.StreamCreate(OUTPUT_RATE, BassFlags.Decode | BassFlags.Float, sourceChannel);
            if (playbackChannel == -1)
            {
                Assert.Fail(string.Format("Failed to create playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.BufferLength, 5);
            BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.Background, true);

            if (!BassAsio.Init(0, AsioInitFlags.Thread))
            {
                Assert.Fail(string.Format("Failed to initialize ASIO: {0}", Enum.GetName(typeof(Errors), BassAsio.LastError)));
            }

            if (!BassAsio.ChannelEnable(false, 0, this.GetAsioProcedure(playbackChannel)))
            {
                Assert.Fail(string.Format("Failed to enable ASIO: {0}", Enum.GetName(typeof(Errors), BassAsio.LastError)));
            }

            if (!BassAsio.ChannelJoin(false, 1, 0))
            {
                Assert.Fail(string.Format("Failed to enable ASIO: {0}", Enum.GetName(typeof(Errors), BassAsio.LastError)));
            }

            if (!BassAsio.ChannelSetRate(false, 0, OUTPUT_RATE))
            {
                Assert.Fail(string.Format("Failed to set ASIO rate: {0}", Enum.GetName(typeof(Errors), BassAsio.LastError)));
            }

            if (!BassAsio.ChannelSetFormat(false, 0, AsioSampleFormat.Float))
            {
                Assert.Fail(string.Format("Failed to set ASIO format: {0}", Enum.GetName(typeof(Errors), BassAsio.LastError)));
            }

            BassAsio.Rate = OUTPUT_RATE;

            if (!BassAsio.Start())
            {
                Assert.Fail(string.Format("Failed to start ASIO: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelLength = Bass.ChannelGetLength(sourceChannel);
            var channelLengthSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelLength);

            do
            {
                if (Bass.ChannelIsActive(sourceChannel) == PlaybackState.Stopped)
                {
                    break;
                }

                var channelPosition = Bass.ChannelGetPosition(sourceChannel);
                var channelPositionSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelPosition);

                Console.WriteLine(
                    "{0}/{1}",
                    TimeSpan.FromSeconds(channelPositionSeconds).ToString("g"),
                    TimeSpan.FromSeconds(channelLengthSeconds).ToString("g")
                );

                Thread.Sleep(1000);
            } while (true);

            if (!BassSox.StreamFree(playbackChannel))
            {
                Assert.Fail(string.Format("Failed to free the playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            if (!Bass.StreamFree(sourceChannel))
            {
                Assert.Fail(string.Format("Failed to free the source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            if (!Bass.Free())
            {
                Assert.Fail(string.Format("Failed to free BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }
        }

        private AsioProcedure GetAsioProcedure(int playbackChannel)
        {
            return new AsioProcedure((Input, Channel, Buffer, Length, User) =>
            {
                return Bass.ChannelGetData(playbackChannel, Buffer, Length);
            });
        }
    }
}
