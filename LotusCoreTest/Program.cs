using System;
using System.IO;
using AtomicAkarin.LotusCore.Common;
using AtomicAkarin.LotusCore.Demux;
using AtomicAkarin.LotusCore.Frames;
using AtomicAkarin.LotusCore.IO;

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

            var fileStream = File.OpenRead(url);
            var io = new StreamIOContext(fileStream);

            using var frameReader = new FrameReaderHandle();
//            frameReader.Open(url);
            frameReader.Open(io.GetIoContext(), url);
            var props = frameReader.GetVideoProperties();
            Console.WriteLine(
                $"FrameReader.GetVideoProperties: {props.CodecName} {props.Width}x{props.Height} {props.PixelFormat}");

            using var scaler = new FrameScalerHandle();
            Console.WriteLine("Scaler.Ctor: ok");

            using var encoder = new FrameEncoderHandle(OutputCodecId, props.Width, props.Height, OutputPixelFormat);
            Console.WriteLine("FrameEncoder.Ctor: ok");

            for (var i = 0; i < 1000; i++)
            {
                using var frame = frameReader.ReadFrame();
                if (i % 100 != 0)
                    continue;
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

                using var stream = new AVPacketStream(packet);
                using var file = File.OpenWrite($"frame-{i}.jpg");
                stream.CopyTo(file);
                file.Flush();
            }
        }
    }
}