using System;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class FrameScaler : IDisposable
    {
        private readonly ScalerContext _context;

        public FrameScaler()
        {
            _context = new ScalerContext();
        }

        public int OutputWidth
        {
            set => _context.OutputWidth = value;
        }

        public int OutputHeight
        {
            set => _context.OutputHeight = value;
        }

        public int OutputPixelFormat
        {
            set => _context.OutputPixelFormat = value;
        }

        public void Dispose()
        {
            Free();
            GC.SuppressFinalize(this);
        }

        // LIBRARY_API(void) scaler_free(ScalerContext *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "scaler_free")]
        private static extern void ScalerFree(ScalerContext ctx);

        // LIBRARY_API(int) scaler_scale(AVFrame *src, AVFrame *out, ScalerContext *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "scaler_scale")]
        private static extern void ScalerScale(IntPtr srcFrame, IntPtr outFrame, ScalerContext ctx);

        private void Free()
        {
            ScalerFree(_context);
        }

        public void Scale(AVFrame sourceFrame, AVFrame outputFrame)
        {
            ScalerScale(sourceFrame.Pointer, outputFrame.Pointer, _context);
        }

        ~FrameScaler()
        {
            Free();
        }

        [StructLayout(LayoutKind.Sequential)]
        private class ScalerContext
        {
            public int OutputWidth;
            public int OutputHeight;
            public int OutputPixelFormat;

            public IntPtr SwsContext;
        }
    }
}