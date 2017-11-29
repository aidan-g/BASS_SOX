using NUnit.Framework;
using System;
using System.Threading;

namespace ManagedBass.Sox.Test
{
    [TestFixture]
    public class Tests
    {
        const int OUTPUT_RATE = 192000;

        [Test]
        public void Test001()
        {
            if (!Bass.Init(Bass.DefaultDevice, OUTPUT_RATE))
            {
                Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var sourceChannel = Bass.CreateStream(@"C:\Source\Prototypes\Resources\1 - 6 - DYE (game version).mp3", 0, 0, BassFlags.Decode | BassFlags.Float);
            if (sourceChannel == -1)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var playbackChannel = BassSox.StreamCreate(OUTPUT_RATE, BassFlags.Float, sourceChannel);
            if (playbackChannel == -1)
            {
                Assert.Fail(string.Format("Failed to create playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            {
                var ok = true;
                ok &= BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.Quality, SoxChannelQuality.VeryHigh);
                ok &= BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.Phase, SoxChannelPhase.Intermediate);
                ok &= BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.SteepFilter, true);
                ok &= BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.AllowAliasing, true);
                ok &= BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.BufferLength, 2);
                ok &= BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.Threads, 2);
                Assert.IsTrue(ok, "Failed to set channel attribute.");
            }

            {
                var ok = true;
                var value = default(int);
                ok &= BassSox.ChannelGetAttribute(playbackChannel, SoxChannelAttribute.Quality, out value);
                Assert.AreEqual((int)SoxChannelQuality.VeryHigh, value);
                ok &= BassSox.ChannelGetAttribute(playbackChannel, SoxChannelAttribute.Phase, out value);
                Assert.AreEqual((int)SoxChannelPhase.Intermediate, value);
                ok &= BassSox.ChannelGetAttribute(playbackChannel, SoxChannelAttribute.SteepFilter, out value);
                Assert.AreEqual(1, value);
                ok &= BassSox.ChannelGetAttribute(playbackChannel, SoxChannelAttribute.AllowAliasing, out value);
                Assert.AreEqual(1, value);
                ok &= BassSox.ChannelGetAttribute(playbackChannel, SoxChannelAttribute.BufferLength, out value);
                Assert.AreEqual(2, value);
                ok &= BassSox.ChannelGetAttribute(playbackChannel, SoxChannelAttribute.Threads, out value);
                Assert.AreEqual(2, value);
                Assert.IsTrue(ok, "Failed to get channel attribute.");
            }

            if (!BassSox.StreamBuffer(playbackChannel))
            {
                Assert.Fail(string.Format("Failed to buffer the playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            if (!Bass.ChannelPlay(playbackChannel))
            {
                Assert.Fail(string.Format("Failed to play the playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var channelLength = Bass.ChannelGetLength(sourceChannel);
            var channelLengthSeconds = Bass.ChannelBytes2Seconds(sourceChannel, channelLength);

            Bass.ChannelSetPosition(sourceChannel, Bass.ChannelSeconds2Bytes(sourceChannel, channelLengthSeconds - 10));

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
    }
}
