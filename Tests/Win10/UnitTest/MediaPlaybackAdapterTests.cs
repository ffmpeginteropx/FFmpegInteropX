using FFmpegInterop;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Graphics.Imaging;
using Windows.Storage;
using Windows.Storage.Streams;
using FFmpegInterop.Helpers;
using Windows.Media.Playback;
using System.Threading;

namespace UnitTest
{
    [TestClass]
    public class MediaPlaybackAdapterTests
    {







        [TestMethod]
        public async Task TestPlaybackStart()
        {
            MediaPlayer player = new MediaPlayer();

            MediaPlaybackItemProvider provider = new MediaPlaybackItemProvider();
            MediaPlaybackListAdapter adapter = new MediaPlaybackListAdapter(provider, await provider.GetPlaybackItem(), await provider.GetPlaybackItem());




            player.Source = adapter.PlaybackList;
            player.Play();

            await Task.Delay(1000);
            await Task.Run(async () =>
            {
                while (player.PlaybackSession.PlaybackState == MediaPlaybackState.Playing)
                {
                    await Task.Delay(1000);
                }
            });

            Assert.IsTrue(adapter.PlaybackList.Items.Count == 2);
        }
    }
}
