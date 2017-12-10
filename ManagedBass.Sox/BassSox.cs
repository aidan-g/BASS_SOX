using System;
using System.Runtime.InteropServices;

namespace ManagedBass.Sox
{
    public enum SoxChannelAttribute
    {
        /// <summary>
        /// <see cref="SoxChannelQuality"/>
        /// </summary>
        Quality = 0,
        /// <summary>
        /// <see cref="SoxChannelPhase"/>
        /// </summary>
        Phase = 1,
        /// <summary>
        /// <see cref="bool"/>
        /// </summary>
        SteepFilter = 2,
        /// <summary>
        /// <see cref="bool"/>
        /// </summary>
        AllowAliasing = 3,
        /// <summary>
        /// <see cref="int"/>
        /// </summary>
        BufferLength = 4,
        /// <summary>
        /// <see cref="int"/>
        /// </summary>
        Threads = 5,
        /// <summary>
        /// <see cref="bool"/>
        /// </summary>
        Background = 6,
        /// <summary>
        /// <see cref="bool"/>
        /// </summary>
        KeepAlive = 7
    }

    public enum SoxChannelQuality
    {
        Quick = 0,
        Low = 1,
        Medium = 2,
        High = Bit20,
        VeryHigh = Bit28,
        Bit16 = 3,
        Bit20 = 4,
        Bit24 = 5,
        Bit28 = 6,
        Bit32 = 7
    }

    public enum SoxChannelPhase
    {
        Linear = 0x00,
        Intermediate = 0x10,
        Minimum = 0x30
    }

    public static class BassSox
    {
        const string DllName = "bass_sox";

        [DllImport(DllName)]
        static extern bool BASS_SOX_Init();

        /// <summary>
        /// Initialize.
        /// </summary>
        /// <returns></returns>
        public static bool Init()
        {
            return BASS_SOX_Init();
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_Free();

        /// <summary>
        /// Free.
        /// </summary>
        /// <returns></returns>
        public static bool Free()
        {
            return BASS_SOX_Free();
        }

        [DllImport(DllName)]
        static extern int BASS_SOX_StreamCreate(int Frequency, BassFlags Flags, int Handle, IntPtr User = default(IntPtr));

        /// <summary>
        /// Create a BASS stream containing a resampler payload for the specified frequency.
        /// </summary>
        /// <param name="Frequency">The target frequency.</param>
        /// <param name="Flags">A combination of <see cref="BassFlags"/>.</param>
        /// <param name="Handle">The stream's handle.</param>
        /// <param name="User">User instance data to pass to the callback function.</param>
        /// <returns>If successful, the new stream's handle is returned, else 0 is returned. Use <see cref="LastError" /> to get the error code.</returns>
        public static int StreamCreate(int Frequency, BassFlags Flags, int Handle, IntPtr User = default(IntPtr))
        {
            return BASS_SOX_StreamCreate(Frequency, Flags, Handle, User);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_StreamBuffer(int Handle);

        /// <summary>
        /// Prepare some data so the stream can play instantly.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <returns></returns>
        public static bool StreamBuffer(int Handle)
        {
            return BASS_SOX_StreamBuffer(Handle);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_StreamBufferClear(int Handle);

        /// <summary>
        /// Clear any buffered data for the stream.
        /// </summary>
        /// <remarks>
        /// Use this when manually changing the stream position.
        /// </remarks>
        /// <param name="Handle">The stream's handle.</param>
        /// <returns></returns>
        public static bool StreamBufferClear(int Handle)
        {
            return BASS_SOX_StreamBufferClear(Handle);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_StreamBufferLength(int Handle, out int Value);

        /// <summary>
        /// Get the length of buffered data. Can be used to calculate the stream position.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <param name="Value"></param>
        /// <returns></returns>
        public static bool StreamBufferLength(int Handle, out int Value)
        {
            return BASS_SOX_StreamBufferLength(Handle, out Value);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_ChannelSetAttribute(int Handle, SoxChannelAttribute Attribute, int Value);

        /// <summary>
        /// Set an attribute on the associated resampler.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <param name="Attribute">A <see cref="SoxChannelAttribute"/>.</param>
        /// <param name="Value"></param>
        /// <returns></returns>
        public static bool ChannelSetAttribute(int Handle, SoxChannelAttribute Attribute, int Value)
        {
            return BASS_SOX_ChannelSetAttribute(Handle, Attribute, Value);
        }

        /// <summary>
        /// Set an attribute on the associated resampler.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <param name="Attribute">A <see cref="SoxChannelAttribute"/>.</param>
        /// <param name="Value">A <see cref="SoxChannelQuality"/>.</param>
        /// <returns></returns>
        public static bool ChannelSetAttribute(int Handle, SoxChannelAttribute Attribute, SoxChannelQuality Value)
        {
            return BASS_SOX_ChannelSetAttribute(Handle, Attribute, (int)Value);
        }

        /// <summary>
        /// Set an attribute on the associated resampler.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <param name="Attribute">A <see cref="SoxChannelAttribute"/>.</param>
        /// <param name="Value">A <see cref="SoxChannelPhase"/>.</param>
        /// <returns></returns>
        public static bool ChannelSetAttribute(int Handle, SoxChannelAttribute Attribute, SoxChannelPhase Value)
        {
            return BASS_SOX_ChannelSetAttribute(Handle, Attribute, (int)Value);
        }

        /// <summary>
        /// Set an attribute on the associated resampler.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <param name="Attribute">A <see cref="SoxChannelAttribute"/>.</param>
        /// <param name="Value"></param>
        /// <returns></returns>
        public static bool ChannelSetAttribute(int Handle, SoxChannelAttribute Attribute, bool Value)
        {
            return BASS_SOX_ChannelSetAttribute(Handle, Attribute, Value ? 1 : 0);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_ChannelGetAttribute(int Handle, SoxChannelAttribute Attribute, out int Value);

        /// <summary>
        /// Get an attribute from the associated resampler.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <param name="Attribute">A <see cref="SoxChannelAttribute"/>.</param>
        /// <param name="Value"></param>
        /// <returns></returns>
        public static bool ChannelGetAttribute(int Handle, SoxChannelAttribute Attribute, out int Value)
        {
            return BASS_SOX_ChannelGetAttribute(Handle, Attribute, out Value);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_StreamFree(int Handle);

        /// <summary>
        /// Release the BASS stream and associated resampler resources.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <returns></returns>
        public static bool StreamFree(int Handle)
        {
            return BASS_SOX_StreamFree(Handle);
        }

        [DllImport(DllName)]
        static extern string BASS_SOX_GetLastError(int Handle);

        /// <summary>
        /// Get the last error encountered by sox.
        /// </summary>
        /// <param name="Handle">The stream's handle.</param>
        /// <returns></returns>
        public static string GetLastError(int Handle)
        {
            return BASS_SOX_GetLastError(Handle);
        }
    }
}
