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
        int openedFiles = 0;
        int numberOfPlaybackItemsToTest = 3;
        bool playbackStarted = false, playbackEnded = false;

        [TestMethod]
        public async Task TestDeferals()
        {
            MediaPlayer player = new MediaPlayer();

            MediaPlaybackListAdapter adapter = new MediaPlaybackListAdapter(await CreateMediaPlaybackItem());
            adapter.PlaybackItemRequest += Adapter_PlaybackItemRequest;
            player.Source = adapter.PlaybackList;
            player.MediaOpened += Player_MediaOpened;
            player.MediaEnded += Player_MediaEnded;
            await adapter.MoveToNextItem();
            await Task.Delay(1000);
            await Task.Run(() =>
            {
                while (!playbackStarted && !playbackEnded)
                {

                }
            });
        }

        private void Player_MediaEnded(MediaPlayer sender, object args)
        {
            playbackEnded = true;

        }

        private void Player_MediaOpened(MediaPlayer sender, object args)
        {
            playbackStarted = true;
        }

        private async void Adapter_PlaybackItemRequest(object sender, MediaPlaybackItemRequestArgs e)
        {
            var deferal = e.GetDeferal();
            var item = await CreateMediaPlaybackItem();
            e.RequestItem = item;
            deferal.Complete();
        }

        private static async Task<MediaPlaybackItem> CreateMediaPlaybackItem()
        {
            var uri = new Uri("ms-appx:///tone.ogg");
            var file = await StorageFile.GetFileFromApplicationUriAsync(uri);
            var stream = await file.OpenAsync(FileAccessMode.Read);

            // CreateFFmpegInteropMSSFromUri should return null if uri is blank with default parameter
            FFmpegInteropMSS FFmpegMSS = await FFmpegInteropMSS.CreateFromStreamAsync(stream);
            return FFmpegMSS.CreateMediaPlaybackItem();
        }


        [TestMethod]
        public async Task TestPlaybackStart()
        {
            MediaPlayer player = new MediaPlayer();
            numberOfPlaybackItemsToTest = 3;
            MediaPlaybackListAdapter adapter = new MediaPlaybackListAdapter(await CreateMediaPlaybackItem());
            adapter.PlaybackItemRequest += Adapter_PlaybackItemRequest2;
            adapter.PlaybackList.CurrentItemChanged += PlaybackList_CurrentItemChanged;

            adapter.PlaybackList.ItemOpened += PlaybackList_ItemOpened;


            player.MediaOpened += Player_MediaOpened;
            player.MediaEnded += Player_MediaEnded;
            player.Source = adapter.PlaybackList;
            player.Play();

            await Task.Delay(1000);
            await Task.Run(async () =>
            {
                while (!playbackStarted && !playbackEnded)
                {
                    await Task.Delay(3000);
                }
            });

            Assert.IsTrue(adapter.PlaybackList.Items.Count == 2);
        }

        private void PlaybackList_ItemOpened(MediaPlaybackList sender, MediaPlaybackItemOpenedEventArgs args)
        {
            if (openedFiles == numberOfPlaybackItemsToTest)
                playbackStarted = true;
            else
            {
                openedFiles++;
            }
        }

        private void PlaybackList_CurrentItemChanged(MediaPlaybackList sender, CurrentMediaPlaybackItemChangedEventArgs args)
        {
            playbackStarted = false;
            playbackEnded = false;
        }





        private async void Adapter_PlaybackItemRequest2(object sender, MediaPlaybackItemRequestArgs e)
        {
            if (openedFiles < numberOfPlaybackItemsToTest)
            {
                var deferal = e.GetDeferal();
                var item = await CreateMediaPlaybackItem();
                e.RequestItem = item;
                deferal.Complete();
            }
        }
    }
}
