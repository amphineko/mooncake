using AtomicAkarin.LotusCore.Handles;

namespace AtomicAkarin.LotusCore.Abstractions
{
    public interface IFrameEncoder
    {
        AVPacketHandle Encode(AVFrameHandle frame);
    }
}