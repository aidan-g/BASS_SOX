using System;
using System.Runtime.InteropServices;

namespace ManagedBass.Sox
{
    public enum SoxChannelAttribute
    {
        Quality = 0,
        Phase = 1,
        SteepFilter = 2,
        AllowAliasing = 3
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
