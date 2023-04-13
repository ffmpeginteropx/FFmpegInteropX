using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage.Streams;

namespace FFmpegInteropX.UnitTests
{
    internal static class Utilities
    {
        public static IRandomAccessStream GetEmbededResourceStream(string path)
        {
            var assembly = Assembly.GetExecutingAssembly();

            Stream stream = assembly.GetManifestResourceStream(path);
            return stream.AsRandomAccessStream();
            
        }
    }
}
