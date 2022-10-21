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

            player.AutoPlay = true;
            var mediaSet = await player.SetMediaAsync(playbackItem);
            Assert.IsTrue(mediaSet);
            player.Play();
            await Task.Delay(5000);
            await player.SeekAsync(TimeSpan.FromSeconds(0));
            //player.Pause();

            //Assert.IsTrue(player.PlaybackSession.Position.Ticks == 0);
            //player.Play();
            //await Task.Delay(5000);
            player.Source = null;
        }
    }
}
