using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.ApplicationModel.Core;
using Windows.ApplicationModel.DataTransfer;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Media.Core;
using Windows.Media.Playback;
using Windows.Storage.AccessCache;
using Windows.Storage.Pickers;
using Windows.Storage.Streams;
using Windows.Storage;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Popups;
using FFmpegInteropX;
using WinRT.Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace MediaPlayerWinUI
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private FFmpegMediaSource FFmpegMSS;
        private FFmpegMediaSource actualFFmpegMSS;
        private StorageFile currentFile;
        private MediaPlaybackItem playbackItem;
        private MediaPlayer mediaPlayer;

        public MainWindow CurrentMainWindow { get; set; }

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
            mediaPlayer.MediaOpened += MediaPlayer_MediaOpened;
            mediaPlayer.MediaFailed += MediaPlayer_MediaFailed;
            mediaPlayerElement.SetMediaPlayer(mediaPlayer);

            CodecChecker.CodecRequired += CodecChecker_CodecRequired;

            // populate character encodings
            cbEncodings.ItemsSource = CharacterEncoding.GetCharacterEncodings();

            AddHandler(KeyDownEvent, new KeyEventHandler(MainPage_KeyDown), true);
        }

        private async void MainPage_KeyDown(object sender, KeyRoutedEventArgs args)
        {
            if (args.Key == VirtualKey.Enter && (Microsoft.UI.Input.InputKeyboardSource.GetKeyStateForCurrentThread(VirtualKey.Control) & CoreVirtualKeyStates.Down)
               == CoreVirtualKeyStates.Down && StorageApplicationPermissions.FutureAccessList.Entries.Count == 1)
            {
                await TryOpenLastFile();
            }

            if (args.Key == VirtualKey.Enter && (Microsoft.UI.Input.InputKeyboardSource.GetKeyStateForCurrentThread(VirtualKey.Shift) & CoreVirtualKeyStates.Down)
               == CoreVirtualKeyStates.Down && StorageApplicationPermissions.FutureAccessList.Entries.Count == 1)
            {
                await TryOpenLastFile();
            }

            if (args.Handled)
            {
                return;
            }

            if (args.Key == VirtualKey.V && !args.Handled)
            {
                if (playbackItem != null && playbackItem.VideoTracks.Count > 1)
                {
                    bool reverse = (Microsoft.UI.Input.InputKeyboardSource.GetKeyStateForCurrentThread(VirtualKey.Control) & CoreVirtualKeyStates.Down) == CoreVirtualKeyStates.Down;
                    int index = reverse ?
                        (playbackItem.VideoTracks.SelectedIndex - 1) % playbackItem.VideoTracks.Count :
                        (playbackItem.VideoTracks.SelectedIndex + 1) % playbackItem.VideoTracks.Count;
                    playbackItem.VideoTracks.SelectedIndex = index;
                }
            }

            if (args.Key == VirtualKey.Right && FFmpegMSS != null && mediaPlayer.PlaybackSession.CanSeek)
            {
                mediaPlayer.PlaybackSession.Position += TimeSpan.FromSeconds(5);
            }

            if (args.Key == VirtualKey.Left && FFmpegMSS != null && mediaPlayer.PlaybackSession.CanSeek)
            {
                mediaPlayer.PlaybackSession.Position -= TimeSpan.FromSeconds(5);
            }
        }

        private void CodecChecker_CodecRequired(object sender, CodecRequiredEventArgs args)
        {
            DispatcherQueue.TryEnqueue(async () =>
                {
                    await AskUserInstallCodec(args);
                });
        }

        private static async Task AskUserInstallCodec(CodecRequiredEventArgs args)
        {
            // show message box to user

            // then open store page
            await args.OpenStorePageAsync();

            // wait for app window to be re-activated
            var tcs = new TaskCompletionSource<object>();
            TypedEventHandler<object, Microsoft.UI.Xaml.WindowActivatedEventArgs> handler = (s, e) =>
            {
                if (e.WindowActivationState != WindowActivationState.Deactivated)
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

        public MediaSourceConfig Config { get; set; }

        private async void OpenLocalFile(object sender, RoutedEventArgs e)
        {
            FileOpenPicker filePicker = new FileOpenPicker();
            InitializeWithWindow.Initialize(filePicker, WindowNative.GetWindowHandle(CurrentMainWindow));

            filePicker.SettingsIdentifier = "VideoFile";
            filePicker.ViewMode = PickerViewMode.Thumbnail;
            filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
            filePicker.FileTypeFilter.Add("*");
            // Show file picker so user can select a file
            StorageFile file = await filePicker.PickSingleFileAsync();

            if (file != null)
            {
                await OpenLocalFile(file);
            }
        }

        private async Task OpenLocalFile(StorageFile file)
        {
            currentFile = file;
            mediaPlayer.Source = null;

            // Open StorageFile as IRandomAccessStream to be passed to FFmpegMediaSource
            IRandomAccessStream readStream = await file.OpenAsync(FileAccessMode.Read);

            try
            {
                StorageApplicationPermissions.FutureAccessList.Clear();
                StorageApplicationPermissions.FutureAccessList.Add(file);

                // Instantiate FFmpegMediaSource using the opened local file stream
                FFmpegMSS = await FFmpegMediaSource.CreateFromStreamAsync(readStream, Config);
                var tags = FFmpegMSS.MetadataTags.ToArray();
                if (AutoCreatePlaybackItem)
                {
                    CreatePlaybackItemAndStartPlaybackInternal();
                }
                else
                {
                    playbackItem = null;
                }
            }
            catch (Exception ex)
            {
                DisplayErrorMessage(ex.Message);
            }
        }

        private void CreatePlaybackItemAndStartPlaybackInternal()
        {
            playbackItem = FFmpegMSS.CreateMediaPlaybackItem();
            mediaPlayer.AutoPlay = true;
            // Pass MediaStreamSource to MediaPlayer
            mediaPlayer.Source = playbackItem;

            // Close control panel after file open
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

                try
                {
                    // Set FFmpeg specific options:
                    // https://www.ffmpeg.org/ffmpeg-protocols.html
                    // https://www.ffmpeg.org/ffmpeg-formats.html

                    // If format cannot be detected, try to increase probesize, max_probe_packets and analyzeduration!

                    // Below are some sample options that you can set to configure RTSP streaming
                    // Config.FFmpegOptions["rtsp_flags"] = "prefer_tcp";
                    Config.FFmpegOptions["stimeout"] = 1000000;
                    Config.FFmpegOptions["timeout"] = 1000000;
                    Config.FFmpegOptions["reconnect"] = 1;
                    Config.FFmpegOptions["reconnect_streamed"] = 1;
                    Config.FFmpegOptions["reconnect_on_network_error"] = 1;

                    // Instantiate FFmpegMediaSource using the URI
                    mediaPlayer.Source = null;
                    FFmpegMSS = await FFmpegMediaSource.CreateFromUriAsync(uri, Config);

                    var source = FFmpegMSS.CreateMediaPlaybackItem();

                    // Pass MediaStreamSource to Media Element
                    mediaPlayer.Source = source;

                    // Close control panel after opening media
                    Splitter.IsPaneOpen = false;
                }
                catch (Exception ex)
                {
                    DisplayErrorMessage(ex.Message);
                }
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
                DisplayErrorMessage("Please open a video file first.");
                return;
            }

            try
            {
                using (frameGrabber)
                using (var frame = await frameGrabber.ExtractVideoFrameAsync(mediaPlayer.PlaybackSession.Position, exactSeek))
                {
                    var filePicker = new FileSavePicker();
                    InitializeWithWindow.Initialize(filePicker, WindowNative.GetWindowHandle(CurrentMainWindow));

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
                            DisplayErrorMessage("File has been created:\n" + file.Path);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                DisplayErrorMessage(ex.Message);
            }
        }

        private async void LoadSubtitleFile(object sender, RoutedEventArgs e)
        {
            if (playbackItem != null)
            {
                FileOpenPicker filePicker = new FileOpenPicker();
                InitializeWithWindow.Initialize(filePicker, WindowNative.GetWindowHandle(CurrentMainWindow));

                filePicker.SettingsIdentifier = "SubtitleFile";
                filePicker.ViewMode = PickerViewMode.Thumbnail;
                filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
                filePicker.FileTypeFilter.Add("*");

                // Show file picker so user can select a file
                StorageFile file = await filePicker.PickSingleFileAsync();

                if (file != null)
                {
                    var track = TimedTextSource.CreateFromStream(await file.OpenReadAsync());
                    playbackItem.Source.ExternalTimedTextSources.Add(track);
                    track.Resolved += Track_Resolved;
                }
            }
            else
            {
                DisplayErrorMessage("Please open a media file before loading an external subtitle for it.");
            }
        }

        private void Track_Resolved(TimedTextSource sender, TimedTextSourceResolveResultEventArgs args)
        {
            // you can rename and pre-select the loaded subtitle track(s) if you like
            var first = args.Tracks.FirstOrDefault();
            if (first != null)
            {
                first.Label = "External";
                var index = playbackItem.TimedMetadataTracks.ToList().IndexOf(first);
                if (index >= 0)
                {
                    playbackItem.TimedMetadataTracks.SetPresentationMode((uint)index, TimedMetadataTrackPresentationMode.PlatformPresented);
                }
            }
        }

        private void MediaPlayer_MediaFailed(MediaPlayer sender, MediaPlayerFailedEventArgs args)
        {
            if (actualFFmpegMSS != null)
            {
                actualFFmpegMSS.Dispose();
                actualFFmpegMSS = null;
                FFmpegMSS = null;
                playbackItem = null;
            }
            var message = args.ErrorMessage;
            if (String.IsNullOrEmpty(message) && args.ExtendedErrorCode != null)
            {
                message = args.ExtendedErrorCode.Message;
            }
            DisplayErrorMessage(message);
        }

        private void DisplayErrorMessage(string message)
        {
            // Display error message
            DispatcherQueue.TryEnqueue(async () =>
            {
                await ShowDialog(message);
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
            if (FFmpegMSS != null)
            {
                try
                {
                    FileOpenPicker filePicker = new FileOpenPicker();
                    var hwnd = WinRT.Interop.WindowNative.GetWindowHandle(CurrentMainWindow);

                    filePicker.SettingsIdentifier = "SubtitleFile";
                    filePicker.ViewMode = PickerViewMode.Thumbnail;
                    filePicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
                    filePicker.FileTypeFilter.Add("*");

                    // Show file picker so user can select a file
                    StorageFile file = await filePicker.PickSingleFileAsync();

                    if (playbackItem != null)
                    {
                        playbackItem.TimedMetadataTracksChanged += PlaybackItem_TimedMetadataTracksChanged;
                    }
                    if (file != null)
                    {
                        var stream = await file.OpenReadAsync();
                        await FFmpegMSS.AddExternalSubtitleAsync(stream, file.Name);
                    }
                }
                catch (Exception ex)
                {
                    DisplayErrorMessage(ex.ToString());
                }
            }
            else
            {
                DisplayErrorMessage("Please open a media file before loading an external subtitle for it.");
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

        private async void CreatePlaybackItemAndStartPlayback(object sender, RoutedEventArgs e)
        {
            if (playbackItem == null)
            {
                CreatePlaybackItemAndStartPlaybackInternal();
                var tracks = playbackItem.TimedMetadataTracks.Count;
            }
            else
            {
                DisplayErrorMessage("Playback item already created.");
            }
        }

        private void PassthroughVideo_Toggled(object sender, RoutedEventArgs e)
        {
            Config.VideoDecoderMode = AutoDetect.IsOn ? VideoDecoderMode.Automatic : PassthroughVideo.IsOn ? VideoDecoderMode.ForceSystemDecoder : VideoDecoderMode.ForceFFmpegSoftwareDecoder;
        }

        private void AddTestFilter(object sender, RoutedEventArgs e)
        {
            if (FFmpegMSS != null)
            {
                FFmpegMSS.SetFFmpegAudioFilters("aecho=0.8:0.9:1000|1800:0.3|0.25");
            }

        }

        private void RemoveTestFilter(object sender, RoutedEventArgs e)
        {
            if (FFmpegMSS != null)
            {
                FFmpegMSS.DisableAudioEffects();
            }
        }

        private void QuickenSubtitles(object sender, RoutedEventArgs e)
        {
            if (FFmpegMSS != null)
            {
                var newOffset = FFmpegMSS.SubtitleDelay.Subtract(TimeSpan.FromSeconds(1));
                FFmpegMSS.SetSubtitleDelay(newOffset);
                tbSubtitleDelay.Text = "Subtitle delay: " + newOffset.TotalSeconds.ToString() + "s";

            }
        }

        private void DelaySubtitles(object sender, RoutedEventArgs e)
        {
            if (FFmpegMSS != null)
            {
                var newOffset = FFmpegMSS.SubtitleDelay.Add(TimeSpan.FromSeconds(1));
                FFmpegMSS.SetSubtitleDelay(newOffset);
                tbSubtitleDelay.Text = "Subtitle delay: " + newOffset.TotalSeconds.ToString() + "s";
            }
        }

        private void MediaPlayer_MediaOpened(MediaPlayer sender, object args)
        {
            var session = sender.PlaybackSession;
            if (session != null && FFmpegMSS != null)
            {
                FFmpegMSS.PlaybackSession = session;
            }
            if (actualFFmpegMSS != null)
            {
                actualFFmpegMSS.Dispose();
            }
            actualFFmpegMSS = FFmpegMSS;
            DispatcherQueue.TryEnqueue(() =>
            {
                tbSubtitleDelay.Text = "Subtitle delay: 0s";
            });

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

        private async Task ShowDialog(string text)
        {
            var dialog = new MessageDialog(text);
            dialog.Title = "An error has occured.";
            InitializeWithWindow.Initialize(dialog, WindowNative.GetWindowHandle(CurrentMainWindow));
            await dialog.ShowAsync();
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
            FFmpegMSS?.SetFFmpegVideoFilters(ffmpegVideoFilters.Text);
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
            FFmpegMSS?.SetFFmpegAudioFilters(ffmpegAudioFilters.Text);
        }
    }
}
