using System;
using System.IO;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class AVPacketStream : Stream
    {
        private readonly IntPtr _pointer;

        public AVPacketStream(AVPacket packet)
        {
            _pointer = packet.Pointer;
        }

        public override bool CanRead => true;

        public override bool CanSeek => true;

        public override bool CanWrite => false;

        public override long Length => ShimPacketLen(_pointer);

        public override long Position { get; set; }

        // LIBRARY_API(int) shim_packet_len(AVPacket *pkt)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_packet_len")]
        private static extern int ShimPacketLen(IntPtr packet);

        // LIBRARY_API(void) shim_packet_read(char *buf, int buf_offset, int read_offset, int read_count, AVPacket *pkt)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_packet_read")]
        private static extern void ShimPacketRead(byte[] buf, int bufOffset, int readOffset, int count, IntPtr pkt);

        public override void Flush()
        {
            // nothing to flush, left blank intentionally
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            var readCount = (int) Math.Min(Length - Position, count);
            ShimPacketRead(buffer, offset, (int) Position, readCount, _pointer);
            return readCount;
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            var length = Length;
            try
            {
                return origin switch
                {
                    SeekOrigin.Begin => (Position = offset),
                    SeekOrigin.Current => (Position += offset),
                    SeekOrigin.End => (Position = length + offset),
                    _ => throw new ArgumentOutOfRangeException(nameof(origin), origin, null)
                };
            }
            finally
            {
                if (Position < 0 || Position >= length)
                    throw new ArgumentOutOfRangeException(nameof(Position), Position, null);
            }
        }

        public override void SetLength(long value)
        {
            throw new NotImplementedException("Resizing underlying AVPacket is not supported");
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new NotImplementedException("Writing to underlying AVPacket is not supported");
        }
    }
}