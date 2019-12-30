using AtomicAkarin.LotusCore.Common;

namespace AtomicAkarin.LotusCore.Frames
{
    public interface IFrameEncoder
    {
        AVPacketHandle Encode(AVFrameHandle frame);
    }
}