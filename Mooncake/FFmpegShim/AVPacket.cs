using System;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class AVPacket : IDisposable
    {
        public AVPacket()
        {
            Pointer = ShimPacketAllocate();
        }

        public IntPtr Pointer { get; }

        public void Dispose()
        {
            Free();
            GC.SuppressFinalize(this);
        }

        // LIBRARY_API(AVPacket *) shim_packet_alloc()
        private static extern IntPtr ShimPacketAllocate();

        // LIBRARY_API(void) shim_packet_free(AVPacket *pkt)
        private static extern void ShimPacketFree(IntPtr packet);

        private void Free()
        {
            ShimPacketFree(Pointer);
        }

        ~AVPacket()
        {
            Free();
        }
    }
}