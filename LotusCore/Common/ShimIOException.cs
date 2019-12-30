using System.IO;

namespace AtomicAkarin.LotusCore.Common
{
    public class ShimIOException : IOException
    {
        public ShimIOException(int errorNumber, string errorString, string step) : base(
            $"{step}: {errorString} ({errorNumber})")
        {
            ErrorNumber = errorNumber;
            ErrorString = errorString;
            Step = step;
        }

        public int ErrorNumber { get; }

        public string ErrorString { get; }

        public string Step { get; }
    }
}