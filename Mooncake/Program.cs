using System;
using System.Threading.Tasks;
using AtomicAkarin.Mooncake.FFmpegShim;

namespace AtomicAkarin.Mooncake
{
    public static class Program
    {
        private static byte[] _parameterSet = Array.Empty<byte>();

        public static async Task Main(string[] args)
        {
            var serverUri = new Uri(args[0]);

            using var frame = new AVFrame();

            using var client = new RtspClient(serverUri.ToString());
            Console.WriteLine("Successfully open RTSP stream");

            Console.WriteLine($"Video Stream Codec={client.GetCodecName()}");

            while (true)
            {
                if (client.ReceiveFrame(frame.Pointer)) ;
                Console.WriteLine("Successfully decoded one frame");
            }
        }
    }
}