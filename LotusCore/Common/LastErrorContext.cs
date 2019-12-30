using System;
using System.Runtime.InteropServices;

namespace AtomicAkarin.LotusCore.Common
{
    [StructLayout(LayoutKind.Sequential)]
    public struct LastErrorContext
    {
        private const int StringSizeConst = 128;

        public readonly int LastError;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = StringSizeConst)]
        public readonly string LastErrorMessage;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = StringSizeConst)]
        public readonly string LastErrorStep;

        public static LastErrorContext FromIntPtr(IntPtr pointer)
        {
            return Marshal.PtrToStructure<LastErrorContext>(pointer);
        }
    }

    public static class LastErrorContextExtensions
    {
        public static void Throw(this LastErrorContext context)
        {
            throw new ShimIOException(context.LastError, context.LastErrorMessage, context.LastErrorStep);
        }
    }
}