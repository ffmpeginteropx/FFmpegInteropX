using Microsoft.VisualStudio.TestTools.UnitTesting;
using FFmpegInteropX;
using System.Threading.Tasks;
using System;

namespace FFmpegInteropX.UnitTests
{
    [TestClass]
    public class CreateDeleteMediaSource
    {
        [TestMethod]
        public async Task CreateDeleteFromUri()
        {
            var source = await FFmpegMediaSource.CreateFromUriAsync("https://samples.mplayerhq.hu/Matroska/subtitles/multiple_sub_sample.mkv");
            Assert.IsNotNull(source);
            source.Dispose();
        }
    }
}
