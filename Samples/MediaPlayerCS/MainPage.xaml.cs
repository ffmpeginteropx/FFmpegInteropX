//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	http ://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
//
//*****************************************************************************

using FFmpegInteropX;

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Windows.ApplicationModel.DataTransfer;
using Windows.ApplicationModel.Core;
using Windows.Foundation.Collections;
using Windows.Media.Core;
using Windows.Media.Playback;
using Windows.Storage;
using Windows.Storage.AccessCache;
using Windows.Storage.Pickers;
using Windows.Storage.Streams;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using System.IO;
using Windows.Foundation;

namespace MediaPlayerCS
{
    public sealed partial class MainPage : Page
    {
        private FFmpegMediaSource CurrentFFmpegMediaSource
        {
            get
            {
                return CurrentPlaybackItem.GetExtradata().MediaSource;
            }
        }

        private StorageFile currentFile;

        private MediaPlaybackItem actualPlaybackItem;
        private MediaPlaybackItem CurrentPlaybackItem
        {
            get
            {
                if (actualPlaybackItem != null)
                    return actualPlaybackItem;

                if (playbackList != null && playbackList.CurrentItem != null)
                    return playbackList.CurrentItem;

                return null;
            }
        }

        private MediaPlaybackList playbackList;

        private MediaPlayer mediaPlayer;

        public bool AutoCreatePlaybackItem
        {
            get;
            set;
        } = true;

        public VideoEffectConfiguration VideoEffectConfiguration
        {
            get;
            set;
        }

        public MainPage()
        {
            Config = new MediaSourceConfig();

            this.InitializeComponent();

            // Show the control panel on startup so user can start opening media
            Splitter.IsPaneOpen = true;
            AutoDetect.IsOn = true;
            VideoEffectConfiguration = new VideoEffectConfiguration();

            mediaPlayer = new MediaPlayer();
            mediaPlayer.AudioCategory = MediaPlayerAudioCategory.Movie;
            mediaPlayer.MediaOpened += SinglePlaybackItem_MediaPlayer_MediaOpened;
            mediaPlayer.MediaFailed += MediaPlayer_MediaFailed;
            mediaPlayerElement.SetMediaPlayer(mediaPlayer);

            CodecChecker.CodecRequired += CodecChecker_CodecRequired;

            // populate character encodings
            cbEncodings.ItemsSource = CharacterEncoding.GetCharacterEncodings();

            CoreWindow.GetForCurrentThread().KeyDown += MainPage_KeyDown;
        }

        private async void MainPage_KeyDown(CoreWindow sender, KeyEventArgs args)
        {
            if (args.VirtualKey == VirtualKey.Enter && (Window.Current.CoreWindow.GetKeyState(VirtualKey.Control) & CoreVirtualKeyStates.Down)
               == CoreVirtualKeyStates.Down && StorageApplicationPermissions.FutureAccessList.Entries.Count == 1)
            {
                await TryOpenLastFile();
            }
            if (args.VirtualKey == VirtualKey.Enter && (Window.Current.CoreWindow.GetKeyState(VirtualKey.Shift) & CoreVirtualKeyStates.Down)
               == CoreVirtualKeyStates.Down && ApplicationData.Current.LocalSettings.Values.ContainsKey("LastUri"))
            {
                await TryOpenLastUri();
            }

            if (args.Handled)
            {
                return;
            }

            if (args.VirtualKey == VirtualKey.V)
            {
                if (CurrentPlaybackItem != null && CurrentPlaybackItem.VideoTracks.Count > 1)
                {
                    bool reverse = (Window.Current.CoreWindow.GetKeyState(VirtualKey.Control) & CoreVirtualKeyStates.Down) == CoreVirtualKeyStates.Down;
                    int index = reverse ?
                        (CurrentPlaybackItem.VideoTracks.SelectedIndex - 1) % CurrentPlaybackItem.VideoTracks.Count :
                        (CurrentPlaybackItem.VideoTracks.SelectedIndex + 1) % CurrentPlaybackItem.VideoTracks.Count;
                    CurrentPlaybackItem.VideoTracks.SelectedIndex = index;
                }
            }

            if (args.VirtualKey == VirtualKey.Right && CurrentFFmpegMediaSource != null && mediaPlayer.PlaybackSession.CanSeek)
            {
                mediaPlayer.PlaybackSession.Position += TimeSpan.FromSeconds(5);
            }

            if (args.VirtualKey == VirtualKey.Left && CurrentFFmpegMediaSource != null && mediaPlayer.PlaybackSession.CanSeek)
            {
                mediaPlayer.PlaybackSession.Position -= TimeSpan.FromSeconds(5);
            }
        }

        private async void CodecChecker_CodecRequired(object sender, CodecRequiredEventArgs args)
        {
            await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(
                CoreDispatcherPriority.Normal,
                new DispatchedHandler(async () =>
                {
                    await AskUserInstallCodec(args);
                }));
        }

        private static async Task AskUserInstallCodec(CodecRequiredEventArgs args)
        {
            // show message box to user

            // then open store page
            await args.OpenStorePageAsync();

            // wait for app window to be re-activated
            var tcs = new TaskCompletionSource<object>();
            WindowActivatedEventHandler handler = (s, e) =>
               {
                   if (e.WindowActivationState != CoreWindowActivationState.Deactivated)
                   {
                       tcs.TrySetResult(null);
                   }
               };
            Window.Current.Activated += handler;
            await tcs.Task;
            Window.Current.Activated -= handler;

            // now refresh codec checker, so next file might use HW acceleration (if codec was really installed)
            await CodecChecker.RefreshAsync();
        }


        private async Task TryOpenLastFile()
        {
            try
            {
                //Try open last file
                var file = await StorageApplicationPermissions.FutureAccessList.GetFileAsync(
                    StorageApplicationPermissions.FutureAccessList.Entries[0].Token);

                await OpenLocalFile(file);
            }
            catch (Exception)
            {
            }
        }
        private async Task TryOpenLastUri()
        {
            try
            {
                //Try open last uri
                var uri = (string)ApplicationData.Current.LocalSettings.Values["LastUri"];
                await OpenStreamUri(uri);
            }
            catch (Exception)
            {
            }
        }

        public MediaSourceConfig Config { get; set; }

        private async void OpenLocalFile(object sender, RoutedEventArgs e)
        {
            FileOpenPicker filePicker = new FileOpenPicker();
            filePicker.SettingsIdentifier = "VideoFile";
            filePicker.ViewMode = PickerViewMode.Thumbnail;
            filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
            filePicker.FileTypeFilter.Add("*");

            // Show file picker so user can select a file
            var files = await filePicker.PickMultipleFilesAsync();

            if (files != null)
            {
                if (files.Count == 1)
                    await OpenLocalFile(files[0]);
                else
                {
                    playbackList = new MediaPlaybackList();
                    foreach (var file in files)
                    {
                        var ffmpegMss = await FFmpegMediaSource.CreateFromStreamAsync(await file.OpenReadAsync());
                        ffmpegMss.PlaybackSession = mediaPlayer.PlaybackSession;
                        var item = ffmpegMss.CreateMediaPlaybackItem();
                        item.Source.CustomProperties.Add(MediaPlaybackItemExtradata.MediaSourceKey, new MediaPlaybackItemExtradata(ffmpegMss));
                        playbackList.Items.Add(item);
                        mediaPlayer.Source = playbackList;
                    }
                }
            }
        }

        private async Task OpenLocalFile(StorageFile file)
        {
            currentFile = file;
            //mediaPlayer.Source = null;

            // Open StorageFile as IRandomAccessStream to be passed to FFmpegMediaSource
            IRandomAccessStream readStream = await file.OpenAsync(FileAccessMode.Read);

            try
            {
                StorageApplicationPermissions.FutureAccessList.Clear();
                StorageApplicationPermissions.FutureAccessList.Add(file);

                // Instantiate FFmpegMediaSource using the opened local file stream
                var mediaSource = await FFmpegMediaSource.CreateFromStreamAsync(readStream, Config);
                var tags = mediaSource.MetadataTags.ToArray();
                if (AutoCreatePlaybackItem)
                {
                    CreatePlaybackItemAndStartPlaybackInternal(mediaSource);
                }
                else
                {
                    actualPlaybackItem = null;
                }
            }
            catch (Exception ex)
            {
                await DisplayErrorMessage(ex.Message);
            }
        }

        private async void CreatePlaybackItemAndStartPlaybackInternal(FFmpegMediaSource mediaSource)
        {
            //playbackItem = FFmpegMSS.CreateMediaPlaybackItem();
            mediaPlayer.AutoPlay = true;
            // Pass MediaStreamSource to MediaPlayer
            //var oldSource = mediaPlayer.Source as MediaPlaybackItem;
            //TaskCompletionSource<bool> playbackItemChanged = new TaskCompletionSource<bool>();
            //TypedEventHandler<MediaPlayer, object> openedEvent = (s, e) =>
            //{
            //    playbackItemChanged.SetResult(true);
            //};
            //TypedEventHandler<MediaPlayer, MediaPlayerFailedEventArgs> failedEvent = (s, e) =>
            //{
            //    playbackItemChanged.SetResult(false);
            //};
            //mediaPlayer.MediaOpened += openedEvent;
            //mediaPlayer.MediaFailed += failedEvent;
            //mediaPlayer.Source = playbackItem;
            //await playbackItemChanged.Task;

            //mediaPlayer.MediaOpened -= openedEvent;
            //mediaPlayer.MediaFailed -= failedEvent;
            //oldSource?.Source?.Dispose();
            // Close control panel after file open


            await mediaSource.OpenWithMediaPlayerAsync(mediaPlayer);
            
            Splitter.IsPaneOpen = false;
        }



        private async void URIBoxKeyUp(object sender, KeyRoutedEventArgs e)
        {
            var textBox = sender as TextBox;
            String uri = textBox.Text;

            // Only respond when the text box is not empty and after Enter key is pressed
            if (e.Key == Windows.System.VirtualKey.Enter && !String.IsNullOrWhiteSpace(uri))
            {
                // Mark event as handled to prevent duplicate event to re-triggered
                e.Handled = true;

                await OpenStreamUri(uri);
            }
        }

        private async Task OpenStreamUri(string uri)
        {
            try
            {
                ApplicationData.Current.LocalSettings.Values["LastUri"] = uri;

                // Set FFmpeg specific options:
                // https://www.ffmpeg.org/ffmpeg-protocols.html
                // https://www.ffmpeg.org/ffmpeg-formats.html

                // If format cannot be detected, try to increase probesize, max_probe_packets and analyzeduration!

                // Below are some sample options that you can set to configure RTSP streaming
                // Config.FFmpegOptions.Add("rtsp_flags", "prefer_tcp");
                Config.FFmpegOptions.Add("stimeout", 1000000);
                Config.FFmpegOptions.Add("timeout", 1000000);
                Config.FFmpegOptions.Add("reconnect", 1);
                Config.FFmpegOptions.Add("reconnect_streamed", 1);
                Config.FFmpegOptions.Add("reconnect_on_network_error", 1);

                // Instantiate FFmpegMediaSource using the URI
                mediaPlayer.Source = null;
                var mediaSource = await FFmpegMediaSource.CreateFromUriAsync(uri, Config);

                if (AutoCreatePlaybackItem)
                {
                    CreatePlaybackItemAndStartPlaybackInternal(mediaSource);
                }
                else
                {
                    actualPlaybackItem = null;
                }
            }
            catch (Exception ex)
            {
                await DisplayErrorMessage(ex.Message);
            }
        }

        private async void ExtractFrame(object sender, RoutedEventArgs e)
        {
            FrameGrabber frameGrabber;
            var uri = tbUri.Text;
            bool exactSeek = grabFrameExactSeek.IsOn;
            if (currentFile != null)
            {
                var stream = await currentFile.OpenAsync(FileAccessMode.Read);
                frameGrabber = await FrameGrabber.CreateFromStreamAsync(stream);
            }
            else if (!string.IsNullOrWhiteSpace(uri))
            {
                frameGrabber = await FrameGrabber.CreateFromUriAsync(uri);
            }
            else
            {
                await DisplayErrorMessage("Please open a video file first.");
                return;
            }

            try
            {
                using (frameGrabber)
                using (var frame = await frameGrabber.ExtractVideoFrameAsync(mediaPlayer.PlaybackSession.Position, exactSeek))
                {
                    var filePicker = new FileSavePicker();
                    filePicker.SettingsIdentifier = "VideoFrame";
                    filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
                    filePicker.DefaultFileExtension = ".jpg";
                    filePicker.FileTypeChoices["Jpeg file"] = new[] { ".jpg" }.ToList();
                    filePicker.FileTypeChoices["Png file"] = new[] { ".png" }.ToList();
                    filePicker.FileTypeChoices["Bmp file"] = new[] { ".bmp" }.ToList();

                    var file = await filePicker.PickSaveFileAsync();
                    if (file != null)
                    {
                        using (var outputStream = await file.OpenAsync(FileAccessMode.ReadWrite))
                        {
                            if (file.FileType == ".jpg")
                            {
                                await frame.EncodeAsJpegAsync(outputStream);
                            }
                            else if (file.FileType == ".png")
                            {
                                await frame.EncodeAsPngAsync(outputStream);
                            }
                            else
                            {
                                await frame.EncodeAsBmpAsync(outputStream);
                            }
                        }

                        bool launched = await Windows.System.Launcher.LaunchFileAsync(file, new LauncherOptions() { DisplayApplicationPicker = false });
                        if (!launched)
                        {
                            await DisplayErrorMessage("File has been created:\n" + file.Path);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                await DisplayErrorMessage(ex.Message);
            }

        }

        private async void LoadSubtitleFile(object sender, RoutedEventArgs e)
        {
            if (CurrentPlaybackItem != null)
            {
                FileOpenPicker filePicker = new FileOpenPicker();
                filePicker.SettingsIdentifier = "SubtitleFile";
                filePicker.ViewMode = PickerViewMode.Thumbnail;
                filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
                filePicker.FileTypeFilter.Add("*");

                // Show file picker so user can select a file
                StorageFile file = await filePicker.PickSingleFileAsync();

                if (file != null)
                {
                    var track = TimedTextSource.CreateFromStream(await file.OpenReadAsync());
                    CurrentPlaybackItem.Source.ExternalTimedTextSources.Add(track);
                    track.Resolved += Track_Resolved;
                }
            }
            else
            {
                await DisplayErrorMessage("Please open a media file before loading an external subtitle for it.");
            }
        }

        private void Track_Resolved(TimedTextSource sender, TimedTextSourceResolveResultEventArgs args)
        {
            // you can rename and pre-select the loaded subtitle track(s) if you like
            var first = args.Tracks.FirstOrDefault();
            if (first != null)
            {
                first.Label = "External";
                var index = CurrentPlaybackItem.TimedMetadataTracks.ToList().IndexOf(first);
                if (index >= 0)
                {
                    CurrentPlaybackItem.TimedMetadataTracks.SetPresentationMode((uint)index, TimedMetadataTrackPresentationMode.PlatformPresented);
                }
            }
        }

        private async void MediaPlayer_MediaFailed(MediaPlayer sender, MediaPlayerFailedEventArgs args)
        {
            await CoreApplication.MainView.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, new DispatchedHandler(
            async () =>
            {
                var message = args.ErrorMessage;
                if (String.IsNullOrEmpty(message) && args.ExtendedErrorCode != null)
                {
                    message = args.ExtendedErrorCode.Message;
                }
                await DisplayErrorMessage(message);
            }));
        }

        private async Task DisplayErrorMessage(string message)
        {
            // Display error message
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                var errorDialog = new MessageDialog(message);
                var x = await errorDialog.ShowAsync();
            });
        }

        private void CbEncodings_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (cbEncodings.SelectedItem != null)
            {
                Config.AnsiSubtitleEncoding = (CharacterEncoding)cbEncodings.SelectedItem;
            }
        }

        private async void LoadSubtitleFileFFmpeg(object sender, RoutedEventArgs e)
        {
            if (CurrentFFmpegMediaSource != null)
            {
                try
                {
                    FileOpenPicker filePicker = new FileOpenPicker();
                    filePicker.SettingsIdentifier = "SubtitleFile";
                    filePicker.ViewMode = PickerViewMode.Thumbnail;
                    filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
                    filePicker.FileTypeFilter.Add("*");

                    // Show file picker so user can select a file
                    StorageFile file = await filePicker.PickSingleFileAsync();

                    if (CurrentPlaybackItem != null)
                    {
                        CurrentPlaybackItem.TimedMetadataTracksChanged += PlaybackItem_TimedMetadataTracksChanged;
                    }
                    if (file != null)
                    {
                        var stream = await file.OpenReadAsync();
                        await CurrentFFmpegMediaSource.AddExternalSubtitleAsync(stream, file.Name);
                    }
                }
                catch (Exception ex)
                {
                    await DisplayErrorMessage(ex.ToString());
                }
            }
            else
            {
                await DisplayErrorMessage("Please open a media file before loading an external subtitle for it.");
            }
        }

        private void PlaybackItem_TimedMetadataTracksChanged(MediaPlaybackItem sender, Windows.Foundation.Collections.IVectorChangedEventArgs args)
        {
            if (args.CollectionChange == Windows.Foundation.Collections.CollectionChange.ItemInserted)
            {
                sender.TimedMetadataTracksChanged -= PlaybackItem_TimedMetadataTracksChanged;

                // unselect other subs
                for (uint i = 0; i < sender.TimedMetadataTracks.Count; i++)
                {
                    sender.TimedMetadataTracks.SetPresentationMode(i, TimedMetadataTrackPresentationMode.Disabled);
                }

                // pre-select added subtitle
                sender.TimedMetadataTracks.SetPresentationMode(args.Index, TimedMetadataTrackPresentationMode.PlatformPresented);
            }
        }

        private void PassthroughVideo_Toggled(object sender, RoutedEventArgs e)
        {
            Config.VideoDecoderMode = AutoDetect.IsOn ? VideoDecoderMode.Automatic : PassthroughVideo.IsOn ? VideoDecoderMode.ForceSystemDecoder : VideoDecoderMode.ForceFFmpegSoftwareDecoder;
        }

        private void AddTestFilter(object sender, RoutedEventArgs e)
        {
            if (CurrentFFmpegMediaSource != null)
            {
                CurrentFFmpegMediaSource.SetFFmpegAudioFilters("aecho=0.8:0.9:1000|1800:0.3|0.25");
            }

        }

        private void RemoveTestFilter(object sender, RoutedEventArgs e)
        {
            if (CurrentFFmpegMediaSource != null)
            {
                CurrentFFmpegMediaSource.DisableAudioEffects();
            }
        }

        private void QuickenSubtitles(object sender, RoutedEventArgs e)
        {
            if (CurrentFFmpegMediaSource != null)
            {
                var newOffset = CurrentFFmpegMediaSource.SubtitleDelay.Subtract(TimeSpan.FromSeconds(1));
                CurrentFFmpegMediaSource.SetSubtitleDelay(newOffset);
                tbSubtitleDelay.Text = "Subtitle delay: " + newOffset.TotalSeconds.ToString() + "s";

            }
        }

        private void DelaySubtitles(object sender, RoutedEventArgs e)
        {
            if (CurrentFFmpegMediaSource != null)
            {
                var newOffset = CurrentFFmpegMediaSource.SubtitleDelay.Add(TimeSpan.FromSeconds(1));
                CurrentFFmpegMediaSource.SetSubtitleDelay(newOffset);
                tbSubtitleDelay.Text = "Subtitle delay: " + newOffset.TotalSeconds.ToString() + "s";
            }
        }

        private async void SinglePlaybackItem_MediaPlayer_MediaOpened(MediaPlayer sender, object args)
        {
            //var session = sender.PlaybackSession;
            //if (session != null && FFmpegMSS != null)
            //{
            //    FFmpegMSS.PlaybackSession = session;
            //}
            //if (actualFFmpegMSS != null)
            //{
            //    //actualFFmpegMSS.Dispose();
            //}
            //actualFFmpegMSS = CurrentFFmpegMediaSource;
            await CoreApplication.MainView.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, new DispatchedHandler(
                () =>
                {
                    tbSubtitleDelay.Text = "Subtitle delay: 0s";
                }));
        }

        private void AutoDetect_Toggled(object sender, RoutedEventArgs e)
        {
            PassthroughVideo.IsEnabled = !AutoDetect.IsOn;
            Config.VideoDecoderMode = AutoDetect.IsOn ? VideoDecoderMode.Automatic : PassthroughVideo.IsOn ? VideoDecoderMode.ForceSystemDecoder : VideoDecoderMode.ForceFFmpegSoftwareDecoder;
        }

        private void EnableVideoEffects_Toggled(object sender, RoutedEventArgs e)
        {
            mediaPlayer.RemoveAllEffects();
            if (enableVideoEffects.IsOn)
            {
                VideoEffectConfiguration.AddVideoEffect(mediaPlayer);
            }
        }

        private void Page_DragEnter(object sender, DragEventArgs e)
        {
            if (e.DataView.Contains(StandardDataFormats.StorageItems))
            {
                e.AcceptedOperation = DataPackageOperation.Link;
            }
        }

        private async void Page_Drop(object sender, DragEventArgs e)
        {
            try
            {
                var files = await e.DataView.GetStorageItemsAsync();
                var first = files.OfType<StorageFile>().FirstOrDefault();
                if (first != null)
                {
                    await OpenLocalFile(first);
                }
            }
            catch
            {
            }
        }

        private void ffmpegVideoFilters_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == VirtualKey.Enter)
            {
                ffmpegVideoFilters_LostFocus(sender, e);
            }
        }

        private void ffmpegVideoFilters_LostFocus(object sender, RoutedEventArgs e)
        {
            Config.FFmpegVideoFilters = ffmpegVideoFilters.Text;
            CurrentFFmpegMediaSource?.SetFFmpegVideoFilters(ffmpegVideoFilters.Text);
        }

        private void ffmpegAudioFilters_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == VirtualKey.Enter)
            {
                ffmpegAudioFilters_LostFocus(sender, e);
            }
        }

        private void ffmpegAudioFilters_LostFocus(object sender, RoutedEventArgs e)
        {
            Config.FFmpegAudioFilters = ffmpegAudioFilters.Text;
            CurrentFFmpegMediaSource?.SetFFmpegAudioFilters(ffmpegAudioFilters.Text);
        }

        private double GetBufferSizeMB()
        {
            return Config.ReadAheadBufferSize / (1024 * 1024);
        }

        private long SetBufferSizeMB(double value)
        {
            return Config.ReadAheadBufferSize = (long)(value * (1024 * 1024));
        }
    }
}
