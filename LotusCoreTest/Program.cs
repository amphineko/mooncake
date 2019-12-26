using System;
using AtomicAkarin.LotusCore;
using AtomicAkarin.LotusCore.Abstractions;
using AtomicAkarin.LotusCore.Handles;

namespace LotusCoreTest
{
    internal static class Program
    {
        private const AVCodecId OutputCodecId = AVCodecId.Png;

        private const AVPixelFormat OutputPixelFormat = AVPixelFormat.RGB24;

        private static void Main(string[] args)
        {
            var url = args[0];
            Console.WriteLine($"Trying Url: {url}");

            using var frameReader = new FrameReaderHandle();
            frameReader.Open(url);
            var props = frameReader.GetVideoProperties();
            Console.WriteLine(
                $"FrameReader.GetVideoProperties: {props.CodecName} {props.Width}x{props.Height} {props.PixelFormat}");

            using var scaler = new FrameScalerHandle();
            Console.WriteLine("Scaler.Ctor: ok");

            using var encoder = new FrameEncoderHandle(OutputCodecId, props.Width, props.Height, OutputPixelFormat);
            Console.WriteLine("FrameEncoder.Ctor: ok");

            for (var i = 0; i < 2; i++)
            {
                using var frame = frameReader.ReadFrame();
                Console.WriteLine("FrameReader.ReadFrame: ok");

                frame.GetProperties(out var width, out var height, out var pixelFormat);
                Console.WriteLine($"SourceFrame.GetProperties: ok {width}x{height} {pixelFormat}");

                using var output = new AVFrameHandle(width, height, OutputPixelFormat);
                Console.WriteLine("OutputFrame.Ctor: ok");

                scaler.Scale(output, frame);
                Console.WriteLine("FrameScaler.Scale: ok");

                output.GetProperties(out var outWidth, out var outHeight, out var outPixelFormat);
                Console.WriteLine($"OutputFrame.GetProperties: ok {outWidth}x{outHeight} {outPixelFormat}");

                using var packet = encoder.Encode(output);
                Console.WriteLine("FrameEncoder.Encode: ok");
            }
        }
    }
}