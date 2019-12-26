using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace AtomicAkarin.LotusCore.Handles
{
    public class AVFrameHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        /// <summary>
        /// LIBRARY_API(AVFrame *) lotus_frame_alloc()
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "lotus_frame_alloc")]
        private static extern IntPtr LotusFrameAllocate();

        /// <summary>
        /// LIBRARY_API(void) lotus_frame_free(AVFrame *frame)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "lotus_frame_free")]
        private static extern IntPtr LotusFrameFree(IntPtr frame);

        /// <summary>
        /// LIBRARY_API(void) lotus_frame_init(int w, int h, int pix_fmt, AVFrame *frame)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "lotus_frame_init")]
        private static extern void LotusFrameInit(int w, int h, int pixFmt, AVFrameHandle frame);

        public AVFrameHandle(IntPtr pointer, bool ownsHandle) : base(ownsHandle)
        {
            SetHandle(pointer);
        }

        public AVFrameHandle() : this(LotusFrameAllocate(), true)
        {
        }

        public AVFrameHandle(int width, int height, AVPixelFormat pixelFormat) : this()
        {
            Initialize(width, height, pixelFormat);
        }

        private void Initialize(int width, int height, AVPixelFormat pixelFormat)
        {
            LotusFrameInit(width, height, (int) pixelFormat, this);
        }

        protected override bool ReleaseHandle()
        {
            LotusFrameFree(handle);
            return true;
        }
    }
}