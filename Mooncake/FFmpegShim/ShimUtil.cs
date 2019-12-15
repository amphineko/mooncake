using System;
using System.Runtime.InteropServices;

namespace AtomicAkarin.Mooncake.FFmpegShim
{
    public class ShimUtil
    {
        public const string LibraryName = "ffmpeg_shim";

        [DllImport(LibraryName, EntryPoint = "shim_free")]
        public static extern void FreeObject(IntPtr pointer);
    }
}