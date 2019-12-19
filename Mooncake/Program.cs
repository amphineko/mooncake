using System;
using System.Threading.Tasks;
using AtomicAkarin.Mooncake.FFmpegShim;

namespace AtomicAkarin.Mooncake
{
    public static class Program
    {
        private static byte[] _parameterSet = Array.Empty<byte>();

        public static Task Main(string[] args)
        {
            var serverUri = new Uri(args[0]);

            using var frame = new AVFrame();
            using var outFrame = new AVFrame(1920, 1080, AVPixelFormat.RGB24);
            using var packet = new AVPacket();

            using var scaler = new FrameScaler();
            scaler.OutputHeight = 1080;
            scaler.OutputWidth = 1920;
            scaler.OutputPixelFormat = (int) AVPixelFormat.RGB24;

            using var encoder = new FrameEncoder((int) AVCodecId.Png, 1920, 1080, AVPixelFormat.RGB24);

            using var client = new RtspClient(serverUri.ToString());
            Console.WriteLine("Successfully open RTSP stream");

            Console.WriteLine($"Video Stream Codec={client.GetCodecName()}");

            while (true)
            {
                if (!client.ReceiveFrame(frame.Pointer))
                    continue;
                Console.WriteLine("Received and decoded one frame");

                scaler.Scale(frame, outFrame);
                Console.WriteLine($"Scaled to {AVPixelFormat.RGB24}");

                encoder.Encode(frame, packet);
                Console.WriteLine("Re-encoded one frame");
            }
        }
    }
}