using System.Threading.Tasks;
using AtomicAkarin.LotusCore.Handles;

namespace AtomicAkarin.LotusCore.Abstractions
{
    public interface IFrameReader
    {
        AVFrameHandle ReadFrame();
    }
}