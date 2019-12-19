using System;
using System.IO;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class FrameEncoder : IDisposable
    {
        private readonly EncoderContext _context;

        public FrameEncoder(int codecId, int width, int height, AVPixelFormat pixelFormat)
        {
            _context = new EncoderContext();
            var ret = BmpContextOpen(_context, codecId, width, height, (int) pixelFormat);
            if (ret != 0)
                throw new IOException($"Failed to open codec {codecId}: Error {ret}");
        }

        public void Dispose()
        {
            Free();
            GC.SuppressFinalize(this);
        }

        // LIBRARY_API(void) bmp_context_free(BmpEncoderContext *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "bmp_context_free")]
        private static extern void BmpContextFree(EncoderContext ctx);

        // LIBRARY_API(int) bmp_context_open(BmpEncoderContext *ctx, int codec_id, int w, int h, int pix_fmt)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "bmp_context_open")]
        private static extern int BmpContextOpen(EncoderContext ctx, int codecId, int w, int h, int pixelFmt);

        // LIBRARY_API(int) bmp_encode(AVFrame *frame, AVPacket *pkt, BmpEncoderContext *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "bmp_encode")]
        private static extern int BmpEncode(IntPtr frame, IntPtr packet, EncoderContext ctx);

        public void Encode(AVFrame frame, AVPacket outputPacket)
        {
            BmpEncode(frame.Pointer, outputPacket.Pointer, _context);
        }

        private void Free()
        {
            BmpContextFree(_context);
        }

        ~FrameEncoder()
        {
            Free();
        }

        [StructLayout(LayoutKind.Sequential)]
        private class EncoderContext
        {
            public IntPtr encoderContext;

            public int LastError;
        }
    }
}