using System;
using System.Runtime.InteropServices;
using AtomicAkarin.LotusCore.Common;
using Microsoft.Win32.SafeHandles;

namespace AtomicAkarin.LotusCore.Frames
{
    public class FrameEncoderHandle : SafeHandleZeroOrMinusOneIsInvalid, IFrameEncoder
    {
        private FrameEncoderHandle() : base(true)
        {
            SetHandle(EncoderContextCreate());
        }

        public FrameEncoderHandle(AVCodecId codecId, int width, int height, AVPixelFormat pixelFormat) : this()
        {
            var ret = EncoderContextOpen((int) codecId, width, height, (int) pixelFormat, this);
            if (ret != 0)
                LastErrorContext.FromIntPtr(handle).Throw();
        }

        public AVPacketHandle Encode(AVFrameHandle frame)
        {
            var packet = new AVPacketHandle();
            try
            {
                var ret = EncoderEncode(packet, frame, this);
                if (ret != 0)
                    LastErrorContext.FromIntPtr(handle).Throw();
                return packet;
            }
            catch
            {
                packet.Dispose();
                throw;
            }
        }

        /// <summary>
        ///     LIBRARY_API(frame_encoder_ctx_t *) encoder_context_create()
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "encoder_context_create")]
        private static extern IntPtr EncoderContextCreate();

        /// <summary>
        ///     LIBRARY_API(void) encoder_context_free(frame_encoder_ctx_t *ctx)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "encoder_context_free")]
        private static extern void EncoderContextClose(IntPtr ctx);

        /// <summary>
        ///     LIBRARY_API(int) encoder_context_open(int codec_id, int w, int h, int pix_fmt, frame_encoder_ctx_t *ctx)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "encoder_context_open")]
        private static extern int EncoderContextOpen(int codecId, int w, int h, int pixFmt, FrameEncoderHandle ctx);

        /// <summary>
        ///     LIBRARY_API(int) encoder_encode(AVPacket *out, AVFrame *in, frame_encoder_ctx_t *ctx)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "encoder_encode")]
        private static extern int EncoderEncode(AVPacketHandle output, AVFrameHandle input, FrameEncoderHandle ctx);

        protected override bool ReleaseHandle()
        {
            EncoderContextClose(handle);
            return true;
        }
    }
}