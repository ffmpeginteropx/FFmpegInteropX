using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Media.Playback;
using Windows.Storage.Pickers;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace MediaPlayerCS
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MediaPlaybackListPage : Page
    {
        MediaPlayer player = new MediaPlayer();
        MediaPlaybackList playbackList;

        MediaPlaybackList PlaybackList
        {
            get => playbackList;
            set
            {
                if (playbackList != null)
                {
                    playbackList.CurrentItemChanged -= PlaybackList_CurrentItemChanged;
                    playbackList.ItemFailed -= PlaybackList_ItemFailed;
                    playbackList.ItemOpened -= PlaybackList_ItemOpened;
                }

                playbackList = value;

                if (playbackList != null)
                {
                    playbackList.CurrentItemChanged += PlaybackList_CurrentItemChanged;
                    playbackList.ItemFailed += PlaybackList_ItemFailed;
                    playbackList.ItemOpened += PlaybackList_ItemOpened;
                }
            }
        }

        private void PlaybackList_ItemOpened(MediaPlaybackList sender, MediaPlaybackItemOpenedEventArgs args)
        {
            System.Diagnostics.Debug.WriteLine("item opened");
        }

        private void PlaybackList_ItemFailed(MediaPlaybackList sender, MediaPlaybackItemFailedEventArgs args)
        {
            System.Diagnostics.Debug.WriteLine("item failed");
        }

        private void PlaybackList_CurrentItemChanged(MediaPlaybackList sender, CurrentMediaPlaybackItemChangedEventArgs args)
        {
            System.Diagnostics.Debug.WriteLine("item changed");
        }

        public MediaPlaybackListPage()
        {
            this.InitializeComponent();
            player.MediaOpened += Player_MediaOpened;
            player.MediaFailed += Player_MediaFailed;
            mediaPlayerElement.SetMediaPlayer(player);
            player.AutoPlay = true;
        }

        private void Player_MediaFailed(MediaPlayer sender, MediaPlayerFailedEventArgs args)
        {
            System.Diagnostics.Debug.WriteLine("Media failed");
        }

        private void Player_MediaOpened(MediaPlayer sender, object args)
        {
            System.Diagnostics.Debug.WriteLine("Media opened");
        }

        private async void OpenMediaItems(object sender, RoutedEventArgs e)
        {
            FileOpenPicker filePicker = new FileOpenPicker();
            filePicker.SettingsIdentifier = "VideoFile";
            filePicker.ViewMode = PickerViewMode.Thumbnail;
            filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
            filePicker.FileTypeFilter.Add("*");

            var files = await filePicker.PickMultipleFilesAsync();
            if (files != null)
            {
                PlaybackList = new MediaPlaybackList();
                foreach (var file in files)
                {
                    var interopMss = await FFmpegInteropX.FFmpegMediaSource.CreateFromStreamAsync(await file.OpenReadAsync());
                    PlaybackList.Items.Add(interopMss.CreateMediaPlaybackItem());
                }

                player.Source = PlaybackList;
            }
        }
    }
}
