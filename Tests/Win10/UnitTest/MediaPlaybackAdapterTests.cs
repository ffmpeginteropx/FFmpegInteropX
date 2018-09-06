using FFmpegInterop;
using FFmpegInterop.Helpers;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Graphics.Imaging;
using Windows.Media.Playback;
using Windows.Storage;
using Windows.Storage.Streams;

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
            MediaPlaybackListAdapter adapter = new MediaPlaybackListAdapter(provider, player);

            await adapter.Start();

            await Task.Delay(1000);
            await Task.Run(async () =>
            {
                while (player.PlaybackSession.PlaybackState == MediaPlaybackState.Playing)
                {
                    await Task.Delay(1000);
                }
            });


        }


    }
}
