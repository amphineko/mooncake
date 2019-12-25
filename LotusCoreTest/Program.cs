using System;
using AtomicAkarin.LotusCore;
using AtomicAkarin.LotusCore.Handles;

namespace LotusCoreTest
{
    internal static class Program
    {
        private static void Main(string[] args)
        {
            var url = args[0];
            Console.WriteLine($"Trying Url: {url}");

            using var frame = new AVFrameHandle();
            if (frame.IsClosed || frame.IsInvalid)
                throw new InvalidOperationException();

            using var frameReader = new FrameReaderHandle();
            frameReader.Open(url);
            var props = frameReader.GetVideoProperties();
            Console.WriteLine($"GetVideoProperties: {props.CodecName} {props.Width}x{props.Height} {props.PixelFormat}");
        }
    }
}