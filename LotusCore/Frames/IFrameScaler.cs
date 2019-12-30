using AtomicAkarin.LotusCore.Common;

namespace AtomicAkarin.LotusCore.Frames
{
    public interface IFrameScaler
    {
        void Scale(AVFrameHandle output, AVFrameHandle input);
    }
}