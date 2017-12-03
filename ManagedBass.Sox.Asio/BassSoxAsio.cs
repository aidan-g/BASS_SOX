using System;
using System.Runtime.InteropServices;

namespace ManagedBass.Sox.Asio
{
    public class BassSoxAsio
    {
        const string DllName = "bass_sox_asio";

        [DllImport(DllName)]
        static extern bool BASS_SOX_ASIO_Init();

        /// <summary>
        /// Initialize.
        /// </summary>
        /// <returns></returns>
        public static bool Init()
        {
            return BASS_SOX_ASIO_Init();
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_ASIO_Free();

        /// <summary>
        /// Free.
        /// </summary>
        /// <returns></returns>
        public static bool Free()
        {
            return BASS_SOX_ASIO_Free();
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_ASIO_StreamGet(out int Handle);

        public static bool StreamGet(out int Handle)
        {
            return BASS_SOX_ASIO_StreamGet(out Handle);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_ASIO_StreamSet(int Handle);

        public static bool StreamSet(int Handle)
        {
            return BASS_SOX_ASIO_StreamSet(Handle);
        }

        [DllImport(DllName)]
        static extern bool BASS_SOX_ASIO_ChannelEnable(bool Input, int Channel, IntPtr User = default(IntPtr));

        public static bool ChannelEnable(bool Input, int Channel, IntPtr User = default(IntPtr))
        {
            return BASS_SOX_ASIO_ChannelEnable(Input, Channel, User);
        }
    }
}
