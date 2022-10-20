using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Threading.Tasks;
using System;
using Windows.Storage.Streams;
using Windows.Storage;
using System.Runtime.InteropServices;

namespace FFmpegInteropX.UnitTests
{
    [TestClass]
    public class FrameGrabberTests
    {
        FrameGrabber grabber;
        IRandomAccessStream fileStream;

        const string UriFile1 = "https://samples.mplayerhq.hu/Matroska/subtitles/multiple_sub_sample.mkv";

        [TestCleanup]
        public void Cleanup()
        {
            grabber?.Dispose();
            grabber = null;
            fileStream?.Dispose();
            fileStream = null;
        }

        [TestMethod]
        public async Task CreateFromUri()
        {
            grabber = await FrameGrabber.CreateFromUriAsync("https://samples.mplayerhq.hu/Matroska/subtitles/multiple_sub_sample.mkv");
            Assert.IsNotNull(grabber);
        }

        [ExpectedException(typeof(COMException))]
        [TestMethod]
        public async Task CreateFromUri404()
        {
            grabber = await FrameGrabber.CreateFromUriAsync(UriFile1 + "NotExists");
        }

        [ExpectedException(typeof(COMException))]
        [TestMethod]
        public async Task CreateFromUriInvalid()
        {
            grabber = await FrameGrabber.CreateFromUriAsync("https://samples.mplayerhq.hu/Matroska/subtitles/");
        }

        //[TestMethod]
        //public async Task CreateFromFile()
        //{
        //    var file = await StorageFile.CreateStreamedFileFromUriAsync("video.mp4", new Uri(UriFile1), null);
        //    fileStream = await file.OpenReadAsync();
        //    grabber = await FrameGrabber.CreateFromStreamAsync(fileStream);
        //    Assert.IsNotNull(grabber);
        //}

        [ExpectedException(typeof(COMException))]
        [TestMethod]
        public async Task CreateFromInvalidFile()
        {
            var file = await StorageFile.GetFileFromPathAsync(System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "FFmpegInteropX.UnitTests.dll"));
            fileStream = await file.OpenReadAsync();
            grabber = await FrameGrabber.CreateFromStreamAsync(fileStream);
        }

        [TestMethod]
        public async Task ExtractNextVideoFrames()
        {
            grabber = await FrameGrabber.CreateFromUriAsync("https://samples.mplayerhq.hu/Matroska/subtitles/multiple_sub_sample.mkv");

            var frame = await grabber.ExtractNextVideoFrameAsync();
            Assert.IsNotNull(frame);
            Assert.IsTrue(frame.Timestamp < TimeSpan.FromSeconds(0.1));
            Assert.IsTrue(frame.DisplayWidth > 0);
            Assert.IsTrue(frame.DisplayHeight > 0);
            Assert.IsTrue(frame.PixelWidth > 0);
            Assert.IsTrue(frame.PixelHeight > 0);
            Assert.IsTrue(frame.DisplayAspectRatio > 0);
            Assert.IsTrue(frame.PixelAspectRatio.Numerator > 0);
            Assert.IsTrue(frame.PixelAspectRatio.Denominator > 0);

            var nextFrame = await grabber.ExtractNextVideoFrameAsync();
            Assert.IsTrue(nextFrame.Timestamp > frame.Timestamp);

            frame.Dispose();
            nextFrame.Dispose();
        }

        [TestMethod]
        public async Task ExtraceSpecificFrameExact()
        {
            grabber = await FrameGrabber.CreateFromUriAsync("https://samples.mplayerhq.hu/Matroska/subtitles/multiple_sub_sample.mkv");

            var frame = await grabber.ExtractVideoFrameAsync(TimeSpan.FromSeconds(4), true);
            Assert.IsNotNull(frame);
            Assert.IsTrue(frame.Timestamp > TimeSpan.FromSeconds(3.9));
            Assert.IsTrue(frame.Timestamp < TimeSpan.FromSeconds(4.1));

            var nextFrame = await grabber.ExtractNextVideoFrameAsync();
            Assert.IsTrue(nextFrame.Timestamp > frame.Timestamp);

            frame.Dispose();
            nextFrame.Dispose();
        }

        [TestMethod]
        public async Task ExtraceSpecificFrameNotExact()
        {
            grabber = await FrameGrabber.CreateFromUriAsync("https://samples.mplayerhq.hu/Matroska/subtitles/multiple_sub_sample.mkv");

            var frame = await grabber.ExtractVideoFrameAsync(TimeSpan.FromSeconds(4), false);
            Assert.IsNotNull(frame);
            Assert.IsTrue(frame.Timestamp < TimeSpan.FromSeconds(3.9));

            var nextFrame = await grabber.ExtractNextVideoFrameAsync();
            Assert.IsTrue(nextFrame.Timestamp > frame.Timestamp);

            frame.Dispose();
            nextFrame.Dispose();
        }
    }
}
