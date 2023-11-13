using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Threading.Tasks;
using System;
using Windows.Storage.Streams;
using Windows.Storage;
using System.Runtime.InteropServices;
using System.Linq;
using System.Reflection;

namespace FFmpegInteropX.UnitTests
{
    [TestClass]
    public class MediaSourceTests
    {
        FFmpegMediaSource source;
        IRandomAccessStream fileStream;
        MediaSourceConfig config;

        const string UriFile1 = "https://samples.mplayerhq.hu/Matroska/subtitles/multiple_sub_sample.mkv";

        [TestInitialize]
        public void Initialize()
        {
            config = new MediaSourceConfig();
            config.Video.VideoDecoderMode = VideoDecoderMode.ForceFFmpegSoftwareDecoder;
        }

        [TestCleanup]
        public void Cleanup()
        {
            source?.Dispose();
            source = null;
            fileStream?.Dispose();
            fileStream = null;
        }

        [TestMethod]
        public async Task CreateFromUri()
        {
            source = await FFmpegMediaSource.CreateFromUriAsync(UriFile1, config);
            Assert.IsNotNull(source);

            Assert.AreEqual(1, source.AudioStreams.Count);
            Assert.AreEqual(1, source.VideoStreams.Count);
            Assert.AreEqual(7, source.SubtitleStreams.Count);

            Assert.IsTrue(source.Duration > TimeSpan.Zero);

            Assert.IsTrue(source.AudioStreams.All(s => !string.IsNullOrEmpty(s.Name)));
            Assert.IsTrue(source.VideoStreams.All(s => !string.IsNullOrEmpty(s.Name)));
            Assert.IsTrue(source.SubtitleStreams.All(s => !string.IsNullOrEmpty(s.Name)));

            Assert.IsTrue(source.MetadataTags.Count > 0);

            Assert.IsFalse(source.HasThumbnail);

            var thumbnail = source.ExtractThumbnail();
            Assert.IsNull(thumbnail);
        }

        [ExpectedException(typeof(COMException))]
        [TestMethod]
        public async Task CreateFromUri404()
        {
            source = await FFmpegMediaSource.CreateFromUriAsync(UriFile1 + "NotExists", config);
        }

        [ExpectedException(typeof(COMException))]
        [TestMethod]
        public async Task CreateFromUriInvalid()
        {
            source = await FFmpegMediaSource.CreateFromUriAsync("https://samples.mplayerhq.hu/Matroska/subtitles/", config);
        }

        [TestMethod]
        public async Task CreateFromFile()
        {
            fileStream = Utilities.GetEmbededResourceStream("FFmpegInteropX.UnitTests.TestFiles.envivio-h264.mp4");
            source = await FFmpegMediaSource.CreateFromStreamAsync(fileStream, config);
            Assert.IsNotNull(source);
        }

        [ExpectedException(typeof(COMException))]
        [TestMethod]
        public async Task CreateFromInvalidFile()
        {
            var file = await StorageFile.GetFileFromPathAsync(Assembly.GetExecutingAssembly().Location);
            fileStream = await file.OpenReadAsync();
            source = await FFmpegMediaSource.CreateFromStreamAsync(fileStream, config);
            Assert.IsNotNull(source);
        }

        [TestMethod]
        public async Task SetFiltersPerStream()
        {
            fileStream = Utilities.GetEmbededResourceStream("FFmpegInteropX.UnitTests.TestFiles.envivio-h264.mp4");

            source = await FFmpegMediaSource.CreateFromStreamAsync(fileStream, config);

            var audioFilter = "equalizer=f=1000:t=h:width=200:g=-10";
            var videoFilter = "blend=all_expr='A*(X/W)+B*(1-X/W)'";

            var audioStreams = source.AudioStreams;
            var videoStreams = source.VideoStreams;

            //set per stream
            source.SetFFmpegAudioFilters(audioFilter, audioStreams.First());
            source.SetFFmpegVideoFilters(videoFilter, videoStreams.First());

            //get per stream
            Assert.AreEqual(audioFilter, source.GetFFmpegAudioFilters(audioStreams.First()));
            Assert.AreEqual(videoFilter, source.GetFFmpegVideoFilters(videoStreams.First()));

            //disable per stream
            source.ClearFFmpegAudioFilters(audioStreams.First());
            source.ClearFFmpegVideoFilters(videoStreams.First());

            //get disabled
            Assert.AreEqual(string.Empty, source.GetFFmpegAudioFilters(audioStreams.First()));
            Assert.AreEqual(string.Empty, source.GetFFmpegVideoFilters(videoStreams.First()));
        }

        [TestMethod]
        public async Task SetFiltersGlobally()
        {
            fileStream = Utilities.GetEmbededResourceStream("FFmpegInteropX.UnitTests.TestFiles.envivio-h264.mp4");

            source = await FFmpegMediaSource.CreateFromStreamAsync(fileStream, config);

            var audioFilter = "equalizer=f=1000:t=h:width=200:g=-10";
            var videoFilter = "blend=all_expr='A*(X/W)+B*(1-X/W)'";

            var audioStreams = source.AudioStreams;
            var videoStreams = source.VideoStreams;

            //set globally
            source.SetFFmpegAudioFilters(audioFilter);
            source.SetFFmpegVideoFilters(videoFilter);

            //get per stream
            Assert.AreEqual(audioFilter, source.GetFFmpegAudioFilters(audioStreams.First()));
            Assert.AreEqual(videoFilter, source.GetFFmpegVideoFilters(videoStreams.First()));

            //disable globally
            source.ClearFFmpegAudioFilters();
            source.ClearFFmpegVideoFilters();

            //get disabled
            Assert.AreEqual(string.Empty, source.GetFFmpegAudioFilters(audioStreams.First()));
            Assert.AreEqual(string.Empty, source.GetFFmpegVideoFilters(videoStreams.First()));
        }

        [TestMethod]
        public async Task SetFiltersFromConfig()
        {
            var audioFilter = "equalizer=f=1000:t=h:width=200:g=-10";
            var videoFilter = "blend=all_expr='A*(X/W)+B*(1-X/W)'";

            config.Audio.FFmpegAudioFilters = audioFilter;
            config.Video.FFmpegVideoFilters = videoFilter;

            fileStream = Utilities.GetEmbededResourceStream("FFmpegInteropX.UnitTests.TestFiles.envivio-h264.mp4");

            source = await FFmpegMediaSource.CreateFromStreamAsync(fileStream, config);

            var audioStreams = source.AudioStreams;
            var videoStreams = source.VideoStreams;

            //get per stream
            Assert.AreEqual(audioFilter, source.GetFFmpegAudioFilters(audioStreams.First()));
            Assert.AreEqual(videoFilter, source.GetFFmpegVideoFilters(videoStreams.First()));
        }
    }
}
