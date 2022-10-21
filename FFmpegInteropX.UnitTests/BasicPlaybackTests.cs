using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Media.Playback;
using Windows.Storage;

namespace FFmpegInteropX.UnitTests
{
    [TestClass]
    public class BasicPlaybackTests
    {
        [TestMethod]
        public async Task PlayMediaTest()
        {
            var fileStream = Utilities.GetEmbededResourceStream("FFmpegInteropX.UnitTests.TestFiles.envivio-h264.mp4");
            using var source = await FFmpegMediaSource.CreateFromStreamAsync(fileStream);

            var playbackItem = source.CreateMediaPlaybackItem();
            Assert.IsNotNull(playbackItem);

            var player = new MediaPlayer();
            player.AutoPlay = false;
            await player.AwaitForMediaOpened(playbackItem);

            player.Play();
            await Task.Delay(1000);
            player.Pause();
            await player.AwaitForSeek(TimeSpan.FromSeconds(0));
            player.Play();
            await Task.Delay(1000);
            player.Pause();
            player.Source = null;
            Assert.IsTrue(player.Position.Ticks == 0);
        }
    }
}
