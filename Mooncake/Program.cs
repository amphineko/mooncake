using System;
using System.IO;
using System.Threading.Tasks;
using AtomicAkarin.Mooncake.FFmpegShim;

namespace AtomicAkarin.Mooncake
{
    public static class Program
    {
        private const AVCodecId OutputCodec = AVCodecId.Png;
        private const int OutputHeight = 1080;
        private const AVPixelFormat OutputPixelFormat = AVPixelFormat.RGB24;
        private const int OutputWidth = 1920;

        private static byte[] _parameterSet = Array.Empty<byte>();

        public static Task Main(string[] args)
        {
            var serverUri = new Uri(args[0]);

            using var scaler = new FrameScaler();
            scaler.OutputHeight = OutputHeight;
            scaler.OutputWidth = OutputWidth;
            scaler.OutputPixelFormat = (int) OutputPixelFormat;

            using var encoder = new FrameEncoder((int) OutputCodec, OutputWidth, OutputHeight, OutputPixelFormat);

            using var client = new RtspClient(serverUri.ToString());
            Console.WriteLine("Successfully open RTSP stream");

            Console.WriteLine($"Remote Video Stream Codec={client.GetCodecName()}");

            using var receivedFrame = new AVFrame();
            while (true)
            {
                if (!client.ReceiveFrame(receivedFrame.Pointer))
                    continue;
                
                var timestamp = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                Console.WriteLine($"{timestamp}: Received and decoded one frame");

                using var frame = new AVFrame();
                scaler.Scale(receivedFrame, frame);
                Console.WriteLine($"{timestamp}: Scaled to {AVPixelFormat.RGB24}");

                using var packet = new AVPacket();
                encoder.Encode(receivedFrame, packet);
                Console.WriteLine($"{timestamp}: Re-encoded one frame");
                
                using var packetStream = new AVPacketStream(packet);
                using var outputFile = File.OpenWrite($"{timestamp}.png");
                packetStream.CopyTo(outputFile);
                outputFile.Flush();
            }
        }
    }
}