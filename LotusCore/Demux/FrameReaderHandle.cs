using System;
using System.Runtime.InteropServices;
using AtomicAkarin.LotusCore.Common;
using AtomicAkarin.LotusCore.Frames;
using Microsoft.Win32.SafeHandles;

namespace AtomicAkarin.LotusCore.Demux
{
    public class FrameReaderHandle : SafeHandleZeroOrMinusOneIsInvalid, IFrameReader
    {
        public FrameReaderHandle() : base(true)
        {
            SetHandle(FrameReaderContextAllocate());
        }

        public AVFrameHandle ReadFrame()
        {
            var frame = new AVFrameHandle();
            try
            {
                var ret = FrameReaderReadFrame(frame, this);
                if (ret != 0)
                    LastErrorContext.FromIntPtr(handle).Throw();
                return frame;
            }
            catch
            {
                frame.Dispose();
                throw;
            }
        }

        public VideoProperties GetVideoProperties()
        {
            var props = new VideoProperties();
            FrameReaderGetVideoProps(out props.Width, out props.Height, out var pixelFormat, out var codecName, handle);
            props.CodecName = Marshal.PtrToStringAnsi(codecName);
            props.PixelFormat = Enum.IsDefined(typeof(AVPixelFormat), pixelFormat)
                ? (AVPixelFormat) pixelFormat
                : AVPixelFormat.Unknown;

            return props;
        }

        public void Open(string uri)
        {
            if (FrameReaderContextOpen(uri, handle) == 0)
                return;

            var error = LastErrorContext.FromIntPtr(handle);
            error.Throw();
        }

        public void Open(IntPtr ioCtx, string uri)
        {
            if (FrameReaderContextOpen2(ioCtx, uri, handle) == 0)
                return;

            var error = LastErrorContext.FromIntPtr(handle);
            error.Throw();
        }

        // LIBRARY_API(frame_reader_ctx_t *) fr_context_alloc()
        [DllImport(ShimUtil.LibraryName, EntryPoint = "fr_context_alloc")]
        private static extern IntPtr FrameReaderContextAllocate();

        // LIBRARY_API(void) fr_context_close(frame_reader_ctx_t *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "fr_context_close")]
        private static extern void FrameReaderContextClose(IntPtr context);

        // LIBRARY_API(int) fr_context_open(char *url, frame_reader_ctx_t *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "fr_context_open")]
        private static extern int FrameReaderContextOpen([MarshalAs(UnmanagedType.LPStr)] string url, IntPtr context);

        // LIBRARY_API(int) fr_context_open2(AVIOContext *ioCtx, char *url, frame_reader_ctx_t *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "fr_context_open2")]
        private static extern int FrameReaderContextOpen2(IntPtr ioCtx, [MarshalAs(UnmanagedType.LPStr)] string url,
            IntPtr context);

        // LIBRARY_API(void) fr_get_video_props(int *w, int *h, int *pix_fmt, char *codec_name, frame_reader_ctx_t *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "fr_get_video_props")]
        private static extern void FrameReaderGetVideoProps(out int w, out int h, out int pixFmt, out IntPtr codecName,
            IntPtr context);

        // LIBRARY_API(int) fr_read_frame(AVFrame *out, frame_reader_ctx_t *ctx)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "fr_read_frame")]
        private static extern int FrameReaderReadFrame(AVFrameHandle frame, FrameReaderHandle context);

        protected override bool ReleaseHandle()
        {
            FrameReaderContextClose(handle);
            return true;
        }
    }
}