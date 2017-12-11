using NUnit.Framework;
using System;
using System.Linq;
using System.Threading;

namespace ManagedBass.Sox.Test
{
    [TestFixture]
    public class Tests
    {
        const int OUTPUT_RATE = 192000;

        const int BASS_STREAMPROC_ERR = -1;

        const int BASS_STREAMPROC_EMPTY = 0;

        const int BASS_STREAMPROC_END = -2147483648; //0x80000000;

        /// <summary>
        /// A basic end to end test.
        /// </summary>
        [Test]
        public void Test001()
        {
            if (!Bass.Init(Bass.DefaultDevice, OUTPUT_RATE))
            {
                Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            if (!BassSox.Init())
            {
                Assert.Fail("Failed to initialize SOX.");
            }

            var sourceChannel = Bass.CreateStream(@"D:\Source\Prototypes\Resources\1 - 6 - DYE (game version).mp3", 0, 0, BassFlags.Decode | BassFlags.Float);
            if (sourceChannel == 0)
            {
                Assert.Fail(string.Format("Failed to create source stream: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }

            var playbackChannel = BassSox.StreamCreate(OUTPUT_RATE, BassFlags.Float, sourceChannel);
            if (playbackChannel == 0)
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

            if (!BassSox.Free())
            {
                Assert.Fail("Failed to free SOX.");
            }

            if (!Bass.Free())
            {
                Assert.Fail(string.Format("Failed to free BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
            }
        }

        /// <summary>
        /// MAX_RESAMPLERS (10) resampler channels are supported.
        /// </summary>
        [Test]
        public void Test002()
        {
            try
            {
                if (!Bass.Init(Bass.NoSoundDevice, 44100))
                {
                    Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
                }

                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channels = new int[10];
                for (var a = 0; a < 10; a++)
                {
                    channels[a] = Bass.CreateStream(44100, 2, BassFlags.Decode, StreamProcedureType.Dummy);
                    Assert.AreNotEqual(0, channels[a]);
                }

                var resamplers = new int[10];
                for (var a = 0; a < 10; a++)
                {
                    resamplers[a] = BassSox.StreamCreate(48000, BassFlags.Decode, channels[a]);
                    Assert.AreNotEqual(0, resamplers[a]);
                }

                for (var a = 0; a < 10; a++)
                {
                    var success = BassSox.StreamFree(resamplers[a]);
                    Assert.IsTrue(success);
                }

                for (var a = 0; a < 10; a++)
                {
                    Bass.StreamFree(channels[a]);
                }
            }
            finally
            {
                BassSox.Free();
                Bass.Free();
            }
        }

        /// <summary>
        /// Invalid source channel is handled.
        /// </summary>
        [Test]
        public void Test003()
        {
            try
            {
                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channel = BassSox.StreamCreate(44100, BassFlags.Decode, 0);
                Assert.AreEqual(0, channel);

                var success = BassSox.StreamFree(0);
                Assert.IsFalse(success);
            }
            finally
            {
                BassSox.Free();
            }
        }

        /// <summary>
        /// Init/Free called out of sequence does not crash.
        /// </summary>
        [Test]
        public void Test004()
        {
            Assert.IsFalse(BassSox.Free());
            Assert.IsTrue(BassSox.Init());
            Assert.IsFalse(BassSox.Init());
            Assert.IsTrue(BassSox.Free());
            Assert.IsFalse(BassSox.Free());
        }

        /// <summary>
        /// Resampling with same input/output rate is ignored.
        /// </summary>
        [Test]
        public void Test005()
        {
            try
            {
                if (!Bass.Init(Bass.NoSoundDevice, 44100))
                {
                    Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
                }

                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channel = Bass.CreateStream(44100, 2, BassFlags.Decode, StreamProcedureType.Dummy);
                var resampler = BassSox.StreamCreate(44100, BassFlags.Decode, channel);
                Assert.AreEqual(channel, resampler);

                Bass.StreamFree(channel);
            }
            finally
            {
                BassSox.Free();
                Bass.Free();
            }
        }

        /// <summary>
        /// Failure to read from source does not mark the stream as "complete" when <see cref="SoxChannelAttribute.KeepAlive"/> is specified.
        /// </summary>
        /// <param name="err"></param>
        //[TestCase(BASS_STREAMPROC_ERR)] //-1 doesn't seem to be understood.
        [TestCase(BASS_STREAMPROC_EMPTY)]
        [TestCase(BASS_STREAMPROC_END)]
        public void Test006(int? err)
        {
            try
            {
                if (!Bass.Init(Bass.NoSoundDevice, 44100))
                {
                    Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
                }

                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channel = Bass.CreateStream(48000, 2, BassFlags.Decode, (handle, buffer, length, user) =>
                {
                    if (err.HasValue)
                    {
                        return err.Value;
                    }
                    return length;
                });

                var resampler = BassSox.StreamCreate(44100, BassFlags.Decode, channel);
                BassSox.ChannelSetAttribute(resampler, SoxChannelAttribute.KeepAlive, true);

                {
                    var buffer = new byte[1024];
                    var length = Bass.ChannelGetData(resampler, buffer, buffer.Length);
                    Assert.AreEqual(0, length);
                }

                err = null;
                Bass.ChannelSetPosition(channel, 0);

                {
                    var buffer = new byte[1024];
                    var length = Bass.ChannelGetData(resampler, buffer, buffer.Length);
                    Assert.AreEqual(buffer.Length, length);
                }

                Bass.StreamFree(channel);
                BassSox.StreamFree(resampler);
            }
            finally
            {
                BassSox.Free();
                Bass.Free();
            }
        }

        /// <summary>
        /// The buffer is populated over time.
        /// </summary>
        [Test]
        public void Test007()
        {
            try
            {
                if (!Bass.Init(Bass.NoSoundDevice, 44100))
                {
                    Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
                }

                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channel = Bass.CreateStream(48000, 2, BassFlags.Decode, (handle, buffer, length, user) => length);

                var resampler = BassSox.StreamCreate(44100, BassFlags.Decode, channel);
                BassSox.ChannelSetAttribute(resampler, SoxChannelAttribute.BufferLength, 5);
                for (var a = 1; a <= 5; a++)
                {
                    var buffer = new byte[1024];
                    var length = Bass.ChannelGetData(resampler, buffer, buffer.Length);
                    if (!BassSox.StreamBufferLength(resampler, out length))
                    {
                        Assert.Fail("Failed to get SOX buffer length.");
                    }
                    Assert.AreEqual(a, length);
                }

                Bass.StreamFree(channel);
                BassSox.StreamFree(resampler);
            }
            finally
            {
                BassSox.Free();
                Bass.Free();
            }
        }

        /// <summary>
        /// The buffer size can be changed while decoding.
        /// </summary>
        [TestCase(1, 5, 10)]
        [TestCase(10, 5, 1)]
        public void Test008(params int[] sizes)
        {
            try
            {
                if (!Bass.Init(Bass.NoSoundDevice, 44100))
                {
                    Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
                }

                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channel = Bass.CreateStream(48000, 2, BassFlags.Decode, (handle, buffer, length, user) => length);

                var resampler = BassSox.StreamCreate(44100, BassFlags.Decode, channel);

                foreach (var size in sizes)
                {
                    BassSox.ChannelSetAttribute(resampler, SoxChannelAttribute.BufferLength, size);
                    for (var a = 1; a <= size + 10; a++)
                    {
                        var buffer = new byte[1024];
                        var length = Bass.ChannelGetData(resampler, buffer, buffer.Length);
                        if (!BassSox.StreamBufferLength(resampler, out length))
                        {
                            Assert.Fail("Failed to get SOX buffer length.");
                        }
                        if (a <= size)
                        {
                            Assert.AreEqual(a, length);
                        }
                        else
                        {
                            Assert.AreEqual(size, length);
                        }
                    }
                }

                Bass.StreamFree(channel);
                BassSox.StreamFree(resampler);
            }
            finally
            {
                BassSox.Free();
                Bass.Free();
            }
        }

        [Test]
        public void Test009()
        {
            try
            {
                if (!Bass.Init(Bass.NoSoundDevice, 44100))
                {
                    Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
                }

                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channel = Bass.CreateStream(48000, 2, BassFlags.Decode, (handle, buffer, length, user) => length);

                var resampler = BassSox.StreamCreate(44100, BassFlags.Decode, channel);

                {
                    var length = default(int);
                    BassSox.StreamBufferLength(resampler, out length);
                    Assert.AreEqual(0, length);
                }

                Bass.StreamFree(channel);
                BassSox.StreamFree(resampler);
            }
            finally
            {
                BassSox.Free();
                Bass.Free();
            }
        }

        /// <summary>
        /// Buffer can be cleared.
        /// </summary>
        [TestCase(null)]    //Should use default.
        [TestCase(0)]       //Should use default.
        [TestCase(1)]
        [TestCase(5)]
        public void Test010(int? bufferLength)
        {
            try
            {
                if (!Bass.Init(Bass.NoSoundDevice, 44100))
                {
                    Assert.Fail(string.Format("Failed to initialize BASS: {0}", Enum.GetName(typeof(Errors), Bass.LastError)));
                }

                if (!BassSox.Init())
                {
                    Assert.Fail("Failed to initialize SOX.");
                }

                var channel = Bass.CreateStream(48000, 2, BassFlags.Decode, (handle, buffer, length, user) => length);

                var resampler = BassSox.StreamCreate(44100, BassFlags.Decode, channel);

                if (bufferLength.HasValue)
                {
                    BassSox.ChannelSetAttribute(resampler, SoxChannelAttribute.BufferLength, bufferLength.Value);
                }

                BassSox.StreamBufferClear(resampler);

                for (var a = 0; a < (bufferLength.HasValue ? bufferLength.Value : 1); a++)
                {
                    var buffer = new byte[1024];
                    var length = Bass.ChannelGetData(resampler, buffer, buffer.Length);
                    Assert.AreEqual(buffer.Length, length);
                }

                BassSox.StreamBufferClear(resampler);

                Bass.StreamFree(channel);
                BassSox.StreamFree(resampler);
            }
            finally
            {
                BassSox.Free();
                Bass.Free();
            }
        }
    }
}
