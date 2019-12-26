using AtomicAkarin.LotusCore.Handles;

namespace AtomicAkarin.LotusCore.Abstractions
{
    public interface IFrameScaler
    {
        void Scale(AVFrameHandle output, AVFrameHandle input);
    }
}