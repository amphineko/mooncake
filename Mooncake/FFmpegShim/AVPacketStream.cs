using System;
using System.IO;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class AVPacketStream : Stream
    {
        private readonly IntPtr _pointer;

        private long _position;

        public AVPacketStream(AVPacket packet)
        {
            _pointer = packet.Pointer;
        }

        public override bool CanRead => true;

        public override bool CanSeek => true;

        public override bool CanWrite => false;

        public override long Length => ShimPacketLen(_pointer);

        public override long Position
        {
            get => _position;
            set
            {
                if (value < 0)
                    throw new IOException("Seeking before zero is illegal");
                if (value >= Length)
                    throw new IOException("Seeking beyond length is not supported");
                _position = value;
            }
        }

        // LIBRARY_API(int) shim_packet_len(AVPacket *pkt)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_packet_len")]
        private static extern int ShimPacketLen(IntPtr packet);

        // LIBRARY_API(void) shim_packet_read(char *buf, int buf_offset, int read_offset, int read_count, AVPacket *pkt)
        [DllImport(ShimUtil.LibraryName, EntryPoint = "shim_packet_read")]
        private static extern void ShimPacketRead(byte[] buf, int bufOffset, int readOffset, int count, IntPtr pkt);

        public override void Flush()
        {
            // nothing to flush, period.
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            if (buffer == null)
                throw new ArgumentNullException(nameof(buffer));
            if (offset < 0)
                throw new ArgumentOutOfRangeException(nameof(offset), offset, null);
            if (count < 0)
                throw new ArgumentOutOfRangeException(nameof(count), count, null);
            if (offset + count > buffer.Length)
                throw new ArgumentException("Invalid offset and count");

            var readCount = (int) Math.Min(Length - Position, count);
            ShimPacketRead(buffer, offset, (int) Position, readCount, _pointer);
            Position += readCount;
            return readCount;
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            return Position = origin switch
            {
                SeekOrigin.Begin => offset,
                SeekOrigin.Current => Position + offset,
                SeekOrigin.End => Length + offset,
                _ => throw new ArgumentOutOfRangeException(nameof(origin), origin, null)
            };
        }

        public override void SetLength(long value)
        {
            throw new IOException("Resizing underlying AVPacket is not supported");
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new IOException("Writing to underlying AVPacket is not supported");
        }
    }
}