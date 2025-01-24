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
using FFmpegInteropX.VideoEffects;

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
using Microsoft.Graphics.Canvas;
using Microsoft.Graphics.Canvas.UI.Xaml;
using Windows.UI;
using Windows.Graphics.Display;
using System.Timers;
using System.Diagnostics;
using System.Threading;

namespace MediaPlayerCS
{
    public sealed partial class MainPage : Page
    {
        private FFmpegMediaSource FFmpegMSS;
        private StorageFile currentFile;
        private MediaPlayer mediaPlayer;
        private TimeSpan subtitleDelay;
        private Image subtitleImage;
        private SubtitleStreamInfo selectedSubtitleStreamInfo;
        private DispatcherTimer subtitleDispatcherTimer = new DispatcherTimer();
        private Stopwatch stopwatch = new Stopwatch();
        private TimeSpan subRender;
        private TimeSpan subPresent;
        private int subFrames;
        private int subErrors;
        private bool bitmapSourceApplied;
        private CanvasImageSource bitmapSource;
        private CanvasDevice device = CanvasDevice.GetSharedDevice();
        private CanvasRenderTarget renderTarget;

        private CanvasSwapChainPanel swapChainPanel;
        private CanvasSwapChain swapChain;

        public bool AutoCreatePlaybackItem
        {
            get;
            set;
        } = true;

        public VideoAdjustmentsConfiguration VideoEffectConfiguration
        {
            get;
            set;
        }

        public MainPage()
        {
            Config = new MediaSourceConfig();
            Config.Subtitles.UseLibassAsSubtitleRenderer = true;
            this.InitializeComponent();

            // Show the control panel on startup so user can start opening media
            Splitter.IsPaneOpen = true;
            AutoDetect.IsOn = true;
            VideoEffectConfiguration = new VideoAdjustmentsConfiguration();

            mediaPlayer = new MediaPlayer();
            mediaPlayer.AudioCategory = MediaPlayerAudioCategory.Movie;
            mediaPlayer.MediaOpened += MediaPlayer_MediaOpened;
            mediaPlayer.MediaFailed += MediaPlayer_MediaFailed;
            mediaPlayerElement.SetMediaPlayer(mediaPlayer);
            mediaPlayer.PlaybackSession.NaturalVideoSizeChanged += PlaybackSession_NaturalVideoSizeChanged;
            CodecChecker.CodecRequired += CodecChecker_CodecRequired;

            // populate character encodings
            cbEncodings.ItemsSource = CharacterEncoding.AllEncodings;

            CoreWindow.GetForCurrentThread().KeyDown += MainPage_KeyDown;

            StreamDelays.AddHandler(Slider.PointerReleasedEvent, new PointerEventHandler(StreamDelayManipulation), true);

            subtitleDispatcherTimer.Interval = TimeSpan.FromMilliseconds(33);
            subtitleDispatcherTimer.Tick += RenderSubtitleDispatcherTimer_Tick;

            mediaPlayer.PlaybackSession.PlaybackStateChanged += PlaybackSession_PlaybackStateChanged;
        }

        private async void PlaybackSession_PlaybackStateChanged(MediaPlaybackSession sender, object args)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                if (sender.PlaybackState == MediaPlaybackState.Playing)
                {
                    if (!isRenderingSubtitles)
                    {
                        StartRenderSubtitles();
                    }
                }
                else
                {
                    if (isRenderingSubtitles)
                    {
                        StopRenderSubtitles();
                    }
                }
            });
        }

        private async void CheckUpdateSubtitleRenderTargets()
        {
            var displayInfo = DisplayInformation.GetForCurrentView();
            var width = Math.Round(mediaPlayerElement.ActualWidth * displayInfo.RawPixelsPerViewPixel);
            var height = Math.Round(mediaPlayerElement.ActualHeight * displayInfo.RawPixelsPerViewPixel);
            if (bitmapSource == null || CheckSizeChanged(bitmapSource.Size, width, height))
            {
                bitmapSource = new CanvasImageSource(device, (float)width, (float)height, 96);
                renderTarget = new CanvasRenderTarget(device, (float)width, (float)height, 96);
                bitmapSourceApplied = false;
            }
        }

        private enum SubtitleRenderTech
        {
            DispatcherTimer,
            DispatcherTimerAsync,
            AsyncLoop
        };

        SubtitleRenderTech renderTech = SubtitleRenderTech.AsyncLoop;
        bool isRenderingSubtitles;
        CancellationTokenSource cancelSubtitlesSource = new CancellationTokenSource();
        Task subtitleLoop = Task.CompletedTask;
        bool useSwapChain = true;

        private void StartRenderSubtitles()
        {
            if (FFmpegMSS != null && FFmpegMSS.SubtitleStreams.Count > 0 && selectedSubtitleStreamInfo != null && renderTarget != null)
            {
                subFrames = 0;
                subErrors = 0;
                subRender = TimeSpan.Zero;
                subPresent = TimeSpan.Zero;
                stopwatch.Restart();

                if (renderTech == SubtitleRenderTech.AsyncLoop)
                {
                    cancelSubtitlesSource = new CancellationTokenSource();
                    subtitleLoop = useSwapChain ? SubtitleRenderLoopSwapChain(cancelSubtitlesSource.Token) : SubtitleRenderLoop(cancelSubtitlesSource.Token);
                }
                else
                {
                    subtitleDispatcherTimer.Start();
                }
                isRenderingSubtitles = true;
            }
        }

        private async void StopRenderSubtitles()
        {
            subtitleDispatcherTimer.Stop();
            cancelSubtitlesSource.Cancel();
            isRenderingSubtitles = false;
            if (renderTech == SubtitleRenderTech.AsyncLoop)
            {
                await subtitleLoop;
            }

            var t1 = stopwatch.Elapsed;
            stopwatch.Stop();
            subtitleDispatcherTimer.Stop();
            var fps = subFrames / t1.TotalSeconds;
            var presentMs = subPresent.TotalMilliseconds / subFrames;
            var renderMs = subRender.TotalMilliseconds / subFrames;
            var stats = new (string, string)[]
            {
                            ("Frames", subFrames.ToString()),
                            ("FramesPerSecond", $"{fps:0.00}"),
                            ("RenderMs", $"{renderMs:0.00}"),
                            ("PresentMs", $"{presentMs:0.00}"),
                            ("Errors", $"{subErrors}"),
            };
            var message = string.Join("\n", stats.Select(s => $"{s.Item1}: {s.Item2}"));
            await new MessageDialog(message, "Subtitle Stats").ShowAsync();
        }

        private async Task SubtitleRenderLoop(CancellationToken token)
        {
            while (!token.IsCancellationRequested)
            {
                try
                {
                    var target = renderTarget;
                    var renderResult = await RenderSubtitleAsync(target);
                    if (target != renderTarget)
                    {
                        continue; // target has changed
                    }

                    var t2 = stopwatch.Elapsed;
                    if (renderResult.Succeeded && renderResult.HasChanged)
                    {
                        using (var ds = bitmapSource.CreateDrawingSession(Colors.Transparent))
                        {
                            ds.DrawImage(renderTarget);
                        }
                    }
                    var t3 = stopwatch.Elapsed;
                    subPresent += t3 - t2;
                    subFrames++;

                    if (!bitmapSourceApplied)
                    {
                        subtitleImage.Source = bitmapSource;
                        bitmapSourceApplied = true;
                    }

                    // without delay, app crashes?!
                    // probably thread context issues?
                    await Task.Delay(5);
                }
                catch
                {
                }
            }
        }


        private async Task SubtitleRenderLoopSwapChain(CancellationToken token)
        {
            while (!token.IsCancellationRequested)
            {
                try
                {
                    var t2 = stopwatch.Elapsed;

                    if ((uint)swapChain.Size.Width != (uint)swapChainPanel.ActualSize.X || (uint)swapChain.Size.Height != (uint)swapChainPanel.ActualSize.Y)
                        swapChain?.ResizeBuffers(new Windows.Foundation.Size((uint)swapChainPanel.ActualSize.X, (uint)swapChainPanel.ActualSize.Y));


                    using (var swapChainDS = swapChain.CreateDrawingSession(Colors.Transparent))
                    {
                        using (var swapChainRenderTarget = new CanvasRenderTarget(swapChainDS, swapChain.Size))
                        {
                            var renderResult = await RenderSubtitleAsync(swapChainRenderTarget);
                            swapChainDS.DrawImage(swapChainRenderTarget);
                            swapChain.Present();
                        }
                    }

                    var t3 = stopwatch.Elapsed;
                    subPresent += t3 - t2;
                    subFrames++;
                }
                catch
                {
                }
            }
        }

        private Task<SubtitleRenderResult> RenderSubtitleAsync(CanvasRenderTarget target)
        {
            return Task.Run(() => RenderSubtitle(target));
        }

        private SubtitleRenderResult RenderSubtitle(CanvasRenderTarget target)
        {
            var t1 = stopwatch.Elapsed;
            var result = FFmpegMSS.RenderSubtitlesToDirectXSurface(target, selectedSubtitleStreamInfo, mediaPlayer.PlaybackSession.Position);
            var t2 = stopwatch.Elapsed;
            subRender += t2 - t1;
            return result;
        }

        private async void RenderSubtitleDispatcherTimer_Tick(object sender, object e)
        {
            if (FFmpegMSS != null && FFmpegMSS.SubtitleStreams.Count > 0 && selectedSubtitleStreamInfo != null && renderTarget != null)
            {
                try
                {
                    SubtitleRenderResult renderResult;
                    var target = renderTarget;
                    if (renderTech == SubtitleRenderTech.DispatcherTimerAsync)
                    {
                        renderResult = await RenderSubtitleAsync(target);
                    }
                    else
                    {
                        renderResult = RenderSubtitle(target);
                    }

                    if (target != renderTarget)
                    {
                        return; // target changed
                    }

                    var t2 = stopwatch.Elapsed;
                    var surfaceChanged = renderResult.Succeeded;

                    if (!surfaceChanged) return;

                    using (var ds = bitmapSource.CreateDrawingSession(Colors.Transparent))
                    {
                        ds.DrawImage(renderTarget);
                    }
                    var t3 = stopwatch.Elapsed;
                    subPresent += t3 - t2;
                    subFrames++;

                    if (!bitmapSourceApplied)
                    {
                        subtitleImage.Source = bitmapSource;
                        bitmapSourceApplied = true;
                    }
                }
                catch (Exception)
                {

                }
            }
        }

        private bool CheckSizeChanged(Windows.Foundation.Size size, double width, double height)
        {
            return Math.Abs(size.Width - width) > 0.1 || Math.Abs(size.Height - height) > 0.1;
        }

        private void PlaybackSession_NaturalVideoSizeChanged(MediaPlaybackSession sender, object args)
        {
            System.Diagnostics.Debug.WriteLine($"Natural video size: {sender.NaturalVideoWidth}x{sender.NaturalVideoHeight}");
        }

        private void StreamDelayManipulation(object sender, PointerRoutedEventArgs e)
        {
            var streamToDelay = cmbAudioVideoStreamDelays.SelectedItem as IStreamInfo;
            if (streamToDelay != null && FFmpegMSS != null)
            {
                FFmpegMSS.SetStreamDelay(streamToDelay, TimeSpan.FromSeconds(StreamDelays.Value));
            }
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

            if (args.VirtualKey == VirtualKey.V && Window.Current.CoreWindow.GetKeyState(VirtualKey.Control) == CoreVirtualKeyStates.None)
            {
                var playbackItem = FFmpegMSS?.PlaybackItem;
                if (playbackItem != null && playbackItem.VideoTracks.Count > 1)
                {
                    bool reverse = (Window.Current.CoreWindow.GetKeyState(VirtualKey.Shift) & CoreVirtualKeyStates.Down) == CoreVirtualKeyStates.Down;
                    int index = reverse ?
                        (playbackItem.VideoTracks.SelectedIndex - 1 + playbackItem.VideoTracks.Count) % playbackItem.VideoTracks.Count : // % is not true modulo!
                        (playbackItem.VideoTracks.SelectedIndex + 1) % playbackItem.VideoTracks.Count;
                    playbackItem.VideoTracks.SelectedIndex = index;
                }
            }

            if (args.VirtualKey == VirtualKey.Right && FFmpegMSS != null && mediaPlayer.PlaybackSession.CanSeek)
            {
                mediaPlayer.PlaybackSession.Position += TimeSpan.FromSeconds(5);
            }

            if (args.VirtualKey == VirtualKey.Left && FFmpegMSS != null && mediaPlayer.PlaybackSession.CanSeek)
            {
                mediaPlayer.PlaybackSession.Position -= TimeSpan.FromSeconds(5);
            }

            if (args.VirtualKey == VirtualKey.Space && FFmpegMSS != null)
            {
                if (mediaPlayer.PlaybackSession.PlaybackState == MediaPlaybackState.Paused)
                {
                    mediaPlayer.Play();
                }
                else
                {
                    mediaPlayer.Pause();
                }
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
            StorageFile file = await filePicker.PickSingleFileAsync();

            if (file != null)
            {
                await OpenLocalFile(file);
            }
        }

        private async Task OpenLocalFile(StorageFile file)
        {
            currentFile = file;

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
            }
            catch (Exception ex)
            {
                await DisplayErrorMessage(ex.Message);
            }
        }

        private async void CreatePlaybackItemAndStartPlaybackInternal()
        {
            mediaPlayer.AutoPlay = true;

            // Open with MediaPlayer
            await FFmpegMSS.OpenWithMediaPlayerAsync(mediaPlayer);

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
                // Config.FFmpegOptions["rtsp_flags"] = "prefer_tcp";
                Config.FFmpegOptions["stimeout"] = 1000000;
                Config.FFmpegOptions["timeout"] = 1000000;
                Config.FFmpegOptions["reconnect"] = 1;
                Config.FFmpegOptions["reconnect_streamed"] = 1;
                Config.FFmpegOptions["reconnect_on_network_error"] = 1;

                // Instantiate FFmpegMediaSource using the URI
                FFmpegMSS = await FFmpegMediaSource.CreateFromUriAsync(uri, Config);
                if (AutoCreatePlaybackItem)
                {
                    CreatePlaybackItemAndStartPlaybackInternal();
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
            var playbackItem = FFmpegMSS?.PlaybackItem;
            if (playbackItem != null)
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
                    playbackItem.Source.ExternalTimedTextSources.Add(track);
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
                var playbackItem = FFmpegMSS?.PlaybackItem;
                if (playbackItem != null)
                {
                    var index = playbackItem.TimedMetadataTracks.ToList().IndexOf(first);
                    if (index >= 0)
                    {
                        playbackItem.TimedMetadataTracks.SetPresentationMode((uint)index, TimedMetadataTrackPresentationMode.PlatformPresented);
                    }
                }
            }
        }

        private async void MediaPlayer_MediaFailed(MediaPlayer sender, MediaPlayerFailedEventArgs args)
        {
            FFmpegMSS = null;
            currentFile = null;
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
                Config.Subtitles.ExternalSubtitleAnsiEncoding = (CharacterEncoding)cbEncodings.SelectedItem;
            }
        }

        private async void LoadSubtitleFileFFmpeg(object sender, RoutedEventArgs e)
        {
            if (FFmpegMSS != null)
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

                    var playbackItem = FFmpegMSS?.PlaybackItem;
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

        private async void CreatePlaybackItemAndStartPlayback(object sender, RoutedEventArgs e)
        {
            var playbackItem = FFmpegMSS?.PlaybackItem;
            if (playbackItem == null)
            {
                CreatePlaybackItemAndStartPlaybackInternal();
            }
            else
            {
                await DisplayErrorMessage("Playback item already created.");
            }
        }

        private void PassthroughVideo_Toggled(object sender, RoutedEventArgs e)
        {
            Config.Video.VideoDecoderMode = AutoDetect.IsOn ? VideoDecoderMode.Automatic : PassthroughVideo.IsOn ? VideoDecoderMode.ForceSystemDecoder : VideoDecoderMode.ForceFFmpegSoftwareDecoder;
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
                FFmpegMSS.ClearFFmpegAudioFilters();
            }
        }

        private void QuickenSubtitles(object sender, RoutedEventArgs e)
        {
            if (FFmpegMSS != null)
            {
                subtitleDelay = subtitleDelay.Subtract(TimeSpan.FromSeconds(1));
                FFmpegMSS.SetSubtitleDelay(subtitleDelay);
                tbSubtitleDelay.Text = "Subtitle delay: " + subtitleDelay.TotalSeconds.ToString() + "s";

            }
        }

        private void DelaySubtitles(object sender, RoutedEventArgs e)
        {
            if (FFmpegMSS != null)
            {
                subtitleDelay = subtitleDelay.Add(TimeSpan.FromSeconds(1));
                FFmpegMSS.SetSubtitleDelay(subtitleDelay);
                tbSubtitleDelay.Text = "Subtitle delay: " + subtitleDelay.TotalSeconds.ToString() + "s";
            }
        }

        private async void MediaPlayer_MediaOpened(MediaPlayer sender, object args)
        {
            var session = sender.PlaybackSession;
            if (session != null && FFmpegMSS != null)
            {
                FFmpegMSS.PlaybackSession = session;
            }
            await CoreApplication.MainView.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, new DispatchedHandler(
                () =>
                {
                    tbSubtitleDelay.Text = "Subtitle delay: 0s";
                    cmbAudioStreamEffectSelector.ItemsSource = FFmpegMSS.AudioStreams;

                    cmbVideoStreamEffectSelector.ItemsSource = FFmpegMSS.VideoStreams;

                    cmbSubtitleSelector.ItemsSource = FFmpegMSS.SubtitleStreams;

                    if (FFmpegMSS.SubtitleStreams.Count > 0)
                        cmbSubtitleSelector.SelectedIndex = 0;

                    List<IStreamInfo> streams = new List<IStreamInfo>();
                    foreach (var a in FFmpegMSS.AudioStreams)
                    {
                        streams.Add(a);
                    }

                    foreach (var vs in FFmpegMSS.VideoStreams)
                    {
                        streams.Add(vs);
                    }

                    cmbAudioVideoStreamDelays.ItemsSource = streams;

                }));
        }

        private void AutoDetect_Toggled(object sender, RoutedEventArgs e)
        {
            PassthroughVideo.IsEnabled = !AutoDetect.IsOn;
            Config.Video.VideoDecoderMode = AutoDetect.IsOn ? VideoDecoderMode.Automatic : PassthroughVideo.IsOn ? VideoDecoderMode.ForceSystemDecoder : VideoDecoderMode.ForceFFmpegSoftwareDecoder;
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
            Config.Video.FFmpegVideoFilters = ffmpegVideoFilters.Text;
            if (cmbVideoStreamEffectSelector.SelectedItem == null)
                FFmpegMSS?.SetFFmpegVideoFilters(ffmpegVideoFilters.Text);
            else FFmpegMSS?.SetFFmpegVideoFilters(ffmpegVideoFilters.Text, (VideoStreamInfo)cmbVideoStreamEffectSelector.SelectedItem);
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
            Config.Audio.FFmpegAudioFilters = ffmpegAudioFilters.Text;
            if (cmbAudioStreamEffectSelector.SelectedItem == null)
                FFmpegMSS?.SetFFmpegAudioFilters(ffmpegAudioFilters.Text);
            else FFmpegMSS?.SetFFmpegAudioFilters(ffmpegAudioFilters.Text, (AudioStreamInfo)cmbAudioStreamEffectSelector.SelectedItem);
        }

        private double GetBufferSizeMB()
        {
            return Config.General.ReadAheadBufferSize / (1024 * 1024);
        }

        private long SetBufferSizeMB(double value)
        {
            return Config.General.ReadAheadBufferSize = (long)(value * (1024 * 1024));
        }

        private void GoToMediaPlaybackListSample(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MediaPlaybackListPage));
        }

        private async void ReadSubtitleFileFFmpeg(object sender, RoutedEventArgs e)
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

                if (file != null)
                {
                    var stream = await file.OpenReadAsync();
                    var subParser = await SubtitleParser.ReadSubtitleAsync(stream);
                    await DisplayErrorMessage($"Subtitle contains {subParser.SubtitleTrack.SubtitleTrack.Cues.Count} cues");
                }
            }
            catch (Exception ex)
            {
                await DisplayErrorMessage(ex.ToString());
            }
        }

        private void SubtitleImage_Loaded(object sender, RoutedEventArgs e)
        {
            subtitleImage = sender as Image;
            CheckUpdateSubtitleRenderTargets();
            SizeChanged += MainPage_SizeChanged;
        }

        private void MainPage_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            CheckUpdateSubtitleRenderTargets();
        }

        private void CmbSubtitleSelector_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (FFmpegMSS != null && FFmpegMSS.SubtitleStreams.Count > 0)
            {
                var item = FFmpegMSS.PlaybackItem;
                if (item != null)
                {
                    for (uint i = 0; i < item.TimedMetadataTracks.Count; i++)
                    {
                        item.TimedMetadataTracks.SetPresentationMode(i, TimedMetadataTrackPresentationMode.Disabled);
                    }
                    if (cmbSubtitleSelector.SelectedIndex != -1)
                    {
                        selectedSubtitleStreamInfo = FFmpegMSS.SubtitleStreams[cmbSubtitleSelector.SelectedIndex];
                        // let us handle the subs
                        item.TimedMetadataTracks.SetPresentationMode((uint)cmbSubtitleSelector.SelectedIndex, TimedMetadataTrackPresentationMode.ApplicationPresented);
                    }
                    else
                    {
                        selectedSubtitleStreamInfo = null;
                    }
                }
            }
        }

        private void getCanvasSwapChainPanel(object sender, RoutedEventArgs e)
        {
            swapChainPanel = sender as CanvasSwapChainPanel;
            device = CanvasDevice.GetSharedDevice();
            swapChainPanel.SwapChain = swapChain = new CanvasSwapChain(device, 400, 400, 96, Windows.Graphics.DirectX.DirectXPixelFormat.R8G8B8A8UIntNormalized, 2, CanvasAlphaMode.Premultiplied);
        }
    }
}
