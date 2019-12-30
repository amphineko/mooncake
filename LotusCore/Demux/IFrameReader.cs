using AtomicAkarin.LotusCore.Common;

namespace AtomicAkarin.LotusCore.Demux
{
    public interface IFrameReader
    {
        AVFrameHandle ReadFrame();
    }
}