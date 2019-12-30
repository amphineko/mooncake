using System;
using System.IO;
using System.Runtime.InteropServices;
using AtomicAkarin.LotusCore.Common;
using Microsoft.Win32.SafeHandles;

namespace AtomicAkarin.LotusCore.IO
{
    public class StreamIOContext : SafeHandleZeroOrMinusOneIsInvalid
    {
        private readonly ReadCallback _readDelegate;

        private readonly SeekCallback _seekDelegate;

        private readonly Stream _stream;

        public StreamIOContext(Stream stream) : base(true)
        {
            _stream = stream;
            // keep references to delegates to prevent garbage collection
            _readDelegate = Read;
            _seekDelegate = Seek;
            SetHandle(ContextAllocate(_readDelegate, _seekDelegate));
        }

        public IntPtr GetIoContext()
        {
            return GetIoContext(this);
        }

        /// <summary>
        ///     LIBRARY_API(StreamIOContext *) sio_alloc(ReadCallback read, SeekCallback seek)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "sio_alloc")]
        private static extern IntPtr ContextAllocate(ReadCallback readCallback, SeekCallback seekCallback);

        /// <summary>
        ///     LIBRARY_API(void) sio_close(StreamIOContext *ctx)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "sio_close")]
        private static extern void ContextClose(IntPtr context);

        /// <summary>
        ///     LIBRARY_API(AVIOContext *) sio_get_io_ctx(StreamIOContext *ctx)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "sio_get_io_ctx")]
        private static extern IntPtr GetIoContext(StreamIOContext context);

        private int Read(byte[] buffer, int bufferSize)
        {
            // TODO: confirm eof behaviour (if should return -1)
            var ret = _stream.Read(buffer, 0, bufferSize);
            return ret == 0 ? -1 : ret;
        }

        private long Seek(long offset, int action)
        {
            if (action == (int) StreamSeekAction.GetSize)
                return _stream.Length;

            var ret = _stream.Seek(offset, action switch
            {
                (int) StreamSeekAction.SeekFromBegin => SeekOrigin.Begin,
                (int) StreamSeekAction.SeekFromCurrent => SeekOrigin.Current,
                (int) StreamSeekAction.SeekFromEnd => SeekOrigin.End,
                _ => throw new IOException("Not implemented seek origin")
            });
            return ret;
        }

        protected override bool ReleaseHandle()
        {
            ContextClose(handle);
            return true;
        }

        private enum StreamSeekAction
        {
            SeekFromCurrent = 1,
            SeekFromBegin = 0,
            SeekFromEnd = 2,
            GetSize = 3
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int ReadCallback([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] [Out]
            byte[] buffer, int bufferSize);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate long SeekCallback(long offset, int action);
    }
}