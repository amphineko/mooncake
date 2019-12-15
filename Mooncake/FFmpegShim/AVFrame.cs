using System;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class AVFrame : IDisposable
    {
        public AVFrame()
        {
            Pointer = ShimAllocateFrame();
        }

        public IntPtr Pointer { get; }

        public void Dispose()
        {
            Free();
            GC.SuppressFinalize(this);
        }

        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_alloc_frame")]
        private static extern IntPtr ShimAllocateFrame();

        private void Free()
        {
            ShimUtil.FreeObject(Pointer);
        }

        ~AVFrame()
        {
            Free();
        }
    }
}