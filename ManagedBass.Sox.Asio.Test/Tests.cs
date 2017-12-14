using ManagedBass.Asio;
using ManagedBass.Sox.Asio;
using NUnit.Framework;
using System;
using System.Threading;

namespace ManagedBass.Sox.Test
{
    public class AsioTests
    {
        const int OUTPUT_RATE = 96000;

        /// <summary>
        /// A basic end to end test.
        /// </summary>
        [Test]
        public void Test001()
        {
            if (!Bass.Init(Bass.NoSoundDevice, OUTPUT_RATE))
            {
                Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var sourceChannel = Bass.CreateStream(@"D:\Source\Prototypes\Resources\1 - 6 - DYE (game version).mp3", 0, 0, BassFlags.Decode | BassFlags.Float);
            if (sourceChannel == 0)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            if (!BassSox.Init())
            {
                Assert.Fail("Failed to initialize SOX.");
            }

            var playbackChannel = BassSox.StreamCreate(OUTPUT_RATE, BassFlags.Decode | BassFlags.Float, sourceChannel);
            if (playbackChannel == 0)
            {
                Assert.Fail(string.Format("Failed to create playback stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.BufferLength, 5);
            BassSox.ChannelSetAttribute(playbackChannel, SoxChannelAttribute.Background, true);

            if (!BassAsio.Init(2, AsioInitFlags.Thread))
            {
                Assert.Fail(string.Format("Failed to initialize ASIO: {0}", Enum.GetName(typeof(Errors), BassAsio.LastError)));
            }

            if (!BassSoxAsio.StreamSet(playbackChannel))
            {
                Assert.Fail("Failed to set ASIO stream.");
            }

            if (!BassSoxAsio.ChannelEnable(false, 0))
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

            BassAsio.Stop();

            BassSox.StreamFree(playbackChannel);
            Bass.StreamFree(sourceChannel);
            BassSox.Free();
            BassSoxAsio.Free();
            BassAsio.Free();
            Bass.Free();
        }

        /// <summary>
        /// Resampler stream handle can be set and retrieved.
        /// </summary>
        [Test]
        public void Test002()
        {
            try
            {
                Bass.Init(Bass.NoSoundDevice, 44100);
                BassSox.Init();
                BassSoxAsio.Init();

                var channel = Bass.CreateStream(44100, 2, BassFlags.Decode, StreamProcedureType.Dummy);
                var resampler = BassSox.StreamCreate(48000, BassFlags.Decode, channel);

                Assert.IsTrue(BassSoxAsio.StreamSet(resampler));

                var retrieved = default(int);
                Assert.IsTrue(BassSoxAsio.StreamGet(out retrieved));
                Assert.AreEqual(resampler, retrieved);

                Assert.IsTrue(BassSox.StreamFree(resampler));
                Assert.IsTrue(Bass.StreamFree(channel));
            }
            finally
            {
                BassSoxAsio.Free();
                BassSox.Free();
                Bass.Free();
            }
        }

        /// <summary>
        /// Cannot set a stream which isn't a resampler.
        /// </summary>
        [Test]
        public void Test003()
        {
            try
            {
                Bass.Init(Bass.NoSoundDevice, 44100);
                BassSox.Init();
                BassSoxAsio.Init();

                var channel = Bass.CreateStream(44100, 2, BassFlags.Decode, StreamProcedureType.Dummy);

                Assert.IsFalse(BassSoxAsio.StreamSet(channel));

                var retrieved = default(int);
                Assert.IsFalse(BassSoxAsio.StreamGet(out retrieved));

                Assert.IsTrue(Bass.StreamFree(channel));
            }
            finally
            {
                BassSoxAsio.Free();
                BassSox.Free();
                Bass.Free();
            }
        }

        /// <summary>
        /// Init/Free called out of sequence does not crash.
        /// </summary>
        [Test]
        public void Test004()
        {
            Assert.IsFalse(BassSoxAsio.Free());
            Assert.IsTrue(BassSoxAsio.Init());
            Assert.IsFalse(BassSoxAsio.Init());
            Assert.IsTrue(BassSoxAsio.Free());
            Assert.IsFalse(BassSoxAsio.Free());
        }
    }
}
