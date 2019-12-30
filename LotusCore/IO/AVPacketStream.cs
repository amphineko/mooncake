using System;
using System.IO;
using System.Runtime.InteropServices;
using AtomicAkarin.LotusCore.Common;

namespace AtomicAkarin.LotusCore.IO
{
    public class AVPacketStream : Stream
    {
        private readonly AVPacketHandle _packet;

        private long _position;

        public AVPacketStream(AVPacketHandle packet)
        {
            _packet = packet;
        }

        public override bool CanRead => true;

        public override bool CanSeek => true;

        public override bool CanWrite => false;

        public override long Length => LotusPacketLen(_packet);

        public override long Position
        {
            get => _position;
            set
            {
                if (value < 0)
                    throw new IOException("Seeking before zero is illegal");
                if (value > Length)
                    throw new IOException("Seeking beyond length is not supported");
                _position = value;
            }
        }

        /// <summary>
        ///     LIBRARY_API(int) lotus_packet_len(AVPacket *pkt)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "lotus_packet_len")]
        private static extern int LotusPacketLen(AVPacketHandle packet);

        /// <summary>
        ///     LIBRARY_API(void) lotus_packet_read(char *buf, int offset, int pos, int count, AVPacket *pkt)
        /// </summary>
        [DllImport(ShimUtil.LibraryName, EntryPoint = "lotus_packet_read")]
        private static extern int LotusPacketRead(byte[] buffer, int offset, int pos, int count, AVPacketHandle pkt);

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
            LotusPacketRead(buffer, offset, (int) _position, readCount, _packet);
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