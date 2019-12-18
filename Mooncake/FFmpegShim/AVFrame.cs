using System;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class AVFrame : IDisposable
    {
        public AVFrame()
        {
            Pointer = ShimFrameAllocate();
        }

        public AVFrame(int width, int height, AVPixelFormat pixelFormat) : this()
        {
            ShimFrameAllocateBuffer(width, height, (int) pixelFormat, Pointer);
        }

        public IntPtr Pointer { get; }

        public void Dispose()
        {
            Free();
            GC.SuppressFinalize(this);
        }

        // LIBRARY_API(AVFrame *) shim_frame_alloc()
        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_frame_alloc")]
        private static extern IntPtr ShimFrameAllocate();

        // LIBRARY_API(int) shim_frame_alloc_buffer(int w, int h, int pix_fmt, AVFrame *frame)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_frame_alloc_buffer")]
        private static extern void ShimFrameAllocateBuffer(int w, int h, int pixFmt, IntPtr frame);

        // LIBRARY_API(void) shim_frame_free(AVFrame *frame)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_frame_free")]
        private static extern void ShimFrameFree(IntPtr frame);

        private void Free()
        {
            ShimFrameFree(Pointer);
        }

        ~AVFrame()
        {
            Free();
        }
    }
}