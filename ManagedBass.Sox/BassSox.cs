using System;
using System.Runtime.InteropServices;

namespace ManagedBass.Sox
{
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
