using System;
using System.Runtime.InteropServices;
using AtomicAkarin.LotusCore.Common;
using Microsoft.Win32.SafeHandles;

namespace AtomicAkarin.LotusCore.Frames
{
    public class FrameScalerHandle : SafeHandleZeroOrMinusOneIsInvalid, IFrameScaler
    {
        public FrameScalerHandle() : base(true)
        {
            SetHandle(ScalerContextAllocate());
        }

        public void Scale(AVFrameHandle output, AVFrameHandle input)
        {
            ScalerScale(output, input, this);
        }

        /// <summary>
        ///     LIBRARY_API(scaler_ctx_t *) scaler_context_alloc()
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "scaler_context_alloc")]
        private static extern IntPtr ScalerContextAllocate();

        /// <summary>
        ///     LIBRARY_API(void) scaler_context_free(scaler_ctx_t *ctx)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "scaler_context_free")]
        private static extern void ScalerContextFree(IntPtr pointer);

        /// <summary>
        ///     LIBRARY_API(int) scaler_scale(AVFrame *out, AVFrame *in, scaler_ctx_t *ctx)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "scaler_scale")]
        private static extern int ScalerScale(AVFrameHandle output, AVFrameHandle input, FrameScalerHandle context);

        protected override bool ReleaseHandle()
        {
            ScalerContextFree(handle);
            return true;
        }
    }
}