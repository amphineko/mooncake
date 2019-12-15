using System;
using System.IO;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    [StructLayout(LayoutKind.Sequential)]
    internal class RtspClientContext
    {
        public IntPtr FormatContext;

        public IntPtr Stream;

        public IntPtr Decoder;
        public IntPtr DecoderContext;

        public int LastError;
    }

    internal class RtspClient : IDisposable
    {
        private const string LibraryName = "ffmpeg_shim";

        private readonly RtspClientContext _context;

        private readonly AVFrame _frame;

        public RtspClient(string uri)
        {
            _context = new RtspClientContext();
            Open(uri);
        }

        public void Dispose()
        {
            Close();
            GC.SuppressFinalize(this);
        }

        [DllImport(LibraryName, EntryPoint = "rtsp_close")]
        private static extern void RtspClose(RtspClientContext ctx);

        [DllImport(LibraryName, EntryPoint = "rtsp_get_codec_name")]
        private static extern IntPtr RtspGetCodecName(RtspClientContext ctx);

        [DllImport(LibraryName, EntryPoint = "rtsp_open")]
        private static extern int RtspOpen([MarshalAs(UnmanagedType.LPStr)] string uri, RtspClientContext ctx);

        [DllImport(LibraryName, EntryPoint = "rtsp_receive_frame")]
        private static extern int RtspReceiveFrame(IntPtr frame, RtspClientContext ctx);

        public string GetCodecName()
        {
            var ptr = RtspGetCodecName(_context);
            return Marshal.PtrToStringAnsi(ptr);
//            var ptr = Marshal.AllocHGlobal(32);
//            try
//            {
//                var len = RtspGetCodecName(ptr, 32, _context);
//                return Marshal.PtrToStringAuto(ptr, Math.Min(len, 32));
//            }
//            finally
//            {
//                Marshal.FreeHGlobal(ptr);
//            }
        }

        private void Open(string uri)
        {
            var ret = RtspOpen(uri, _context);
            if (ret == 0)
                return;

            throw ret switch
            {
                -1 => (Exception) new IOException($"Failed to open RTSP stream ({(PosixError) ret})"),
                -2 => new InvalidDataException($"Failed to retrieve stream info ({(PosixError) ret})"),
                -3 => new InvalidDataException("No candidate video stream from RTSP source"),
//                -4 => new IOException($"Failed to copy decoder parameters ({(PosixError) ret})"),
                -5 => new IOException($"Failed to open decoder ({ret})"),
                _ => new NotImplementedException($"Unexpected error ({ret})")
            };
        }

        public bool ReceiveFrame(IntPtr framePtr)
        {
            var ret = RtspReceiveFrame(framePtr, _context);
            return ret switch
            {
                0 => true,
                (int) PosixError.Again => false,
                (int) PosixError.Invalid => throw new IOException("Decoder is currently in invalid state"),
                _ => throw new NotImplementedException($"Unexpected error ({ret})")
            };
        }

        private void Close()
        {
            RtspClose(_context);
        }

        ~RtspClient()
        {
            Close();
        }
    }
}