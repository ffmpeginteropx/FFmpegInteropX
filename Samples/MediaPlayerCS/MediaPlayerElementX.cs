using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Media.Playback;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Shapes;
using Microsoft.Graphics.Canvas;
using Microsoft.Graphics.Canvas.Brushes;
using Microsoft.Graphics.Canvas.Text;
using Microsoft.Graphics.Canvas.UI.Xaml;
using Windows.Graphics.Imaging;
using Windows.UI.Core;
using Windows.Media.Core;
using Windows.UI;
using FFmpegInteropX;

namespace MediaPlayerCS
{
    internal class MediaPlayerElementX : Control
    {

        SubtitleRenderer subRenderer = new SubtitleRenderer();

        public Image FrameServerImage
        {
            get;
            private set;
        }

        public Image SubtitleTexture
        {
            get;
            private set;
        }

        public Rectangle MediaPlayerPresenter
        {
            get;
            private set;
        }

        MediaPlayer _mediaPlayer;

        private MediaPlayer WrappedMediaPlayer
        {
            get => _mediaPlayer;
            set
            {
                if (_mediaPlayer != value)
                {
                    if (_mediaPlayer != null)
                    {
                        _mediaPlayer.VideoFrameAvailable -= _mediaPlayer_VideoFrameAvailable;
                        _mediaPlayer.IsVideoFrameServerEnabled = false;
                    }
                    _mediaPlayer = value;
                    if (_mediaPlayer != null)
                    {
                        _mediaPlayer.VideoFrameAvailable += _mediaPlayer_VideoFrameAvailable;
                        _mediaPlayer.SubtitleFrameChanged += _mediaPlayer_SubtitleFrameChanged;
                        _mediaPlayer.IsVideoFrameServerEnabled = true;
                        _mediaPlayer.SetSurfaceSize(new Size(1920, 1080));
                    }
                }
            }
        }

        private void _mediaPlayer_SubtitleFrameChanged(MediaPlayer sender, object args)
        {
            _ = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                RenderSubtitleFrame(sender, true);
            });
        }

        public Stretch Stretch { get; set; }

        public ImageSource PosterSource { get; set; }

        public bool AreTransportControlsEnabled { get; set; }

        public static DependencyProperty StretchProperty = DependencyProperty.Register(nameof(Stretch), typeof(Stretch), typeof(MediaPlayerElementX), new PropertyMetadata(Stretch.None, new PropertyChangedCallback(StretchPropertyChanged)));
        public static DependencyProperty PosterSourceProperty = DependencyProperty.Register(nameof(PosterSource), typeof(Stretch), typeof(MediaPlayerElementX), new PropertyMetadata(Stretch.None, new PropertyChangedCallback(PosterSourcePropertyChanged)));
        public static DependencyProperty AreTransportControlsEnabledProperty = DependencyProperty.Register(nameof(AreTransportControlsEnabled), typeof(Stretch), typeof(MediaPlayerElementX), new PropertyMetadata(Stretch.None, new PropertyChangedCallback(AreTransportControlsEnabledPropertyChanged)));

        private static void AreTransportControlsEnabledPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as MediaPlayerElementX).AreTransportControlsEnabled = (bool)e.NewValue;
        }

        private static void PosterSourcePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as MediaPlayerElementX).PosterSource = e.NewValue as ImageSource;
        }

        private static void StretchPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as MediaPlayerElementX).Stretch = (Stretch)e.NewValue;
        }

        public void SetMediaPlayer(MediaPlayer player)
        {
            WrappedMediaPlayer = player;
        }

        private SoftwareBitmap frameServerDest;
        private CanvasImageSource canvasImageSource;

        private SoftwareBitmap subtitleDest;
        private CanvasImageSource subtitleImageSource;

        private long mediaPlayerchangedToken;
        App app;
        private CanvasDevice canvasDevice;

        public MediaPlayerElementX()
        {
            this.DefaultStyleKey = typeof(MediaPlayerElementX);
            app = App.Current as App;
        }

        protected override void OnApplyTemplate()
        {
            FrameServerImage = GetTemplateChild("PosterImage2") as Image;
            FrameServerImage.Unloaded += FrameServerImage_Unloaded;
            FrameServerImage.Visibility = Visibility.Visible;
            FrameServerImage.ImageFailed += FrameServerImage_ImageFailed;
            MediaPlayerPresenter = GetTemplateChild("MediaPlayerPresenter") as Rectangle;
            SubtitleTexture = GetTemplateChild("SubtitlesTexture") as Image;
            base.OnApplyTemplate();
        }

        private void FrameServerImage_ImageFailed(object sender, ExceptionRoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void FrameServerImage_Unloaded(object sender, RoutedEventArgs e)
        {

        }


        private async void _mediaPlayer_VideoFrameAvailable(MediaPlayer sender, object args)
        {
            _ = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                RenderVideoFrame(sender, false);
            });
        }

        private void RenderVideoFrame(MediaPlayer sender, bool drawSubs)
        {
            try
            {
                canvasDevice = CanvasDevice.GetSharedDevice();

                if (sender.PlaybackSession.PlaybackState == MediaPlaybackState.Paused) return;
                FrameServerImage.Width = MediaPlayerPresenter.ActualWidth;
                FrameServerImage.Height = MediaPlayerPresenter.ActualHeight;
                //if (!Window.Current.Visible || FrameServerImage.Width == 0 || FrameServerImage.Height == 0) return;
                FrameServerImage.Visibility = Visibility.Visible;
                FrameServerImage.Opacity = 1;
                this.Visibility = Visibility.Visible;
                if (frameServerDest == null || (frameServerDest.PixelWidth != FrameServerImage.Width) || (frameServerDest.PixelHeight != FrameServerImage.Height))
                {
                    frameServerDest?.Dispose();
                    // FrameServerImage in this example is a XAML image control
                    frameServerDest = new SoftwareBitmap(BitmapPixelFormat.Bgra8, (int)FrameServerImage.Width, (int)FrameServerImage.Height, BitmapAlphaMode.Premultiplied);
                }
                if (canvasImageSource == null || (canvasImageSource.Size.Width != FrameServerImage.Width) || (canvasImageSource.Size.Height != FrameServerImage.Height))
                {
                    canvasImageSource = new CanvasImageSource(canvasDevice, (int)FrameServerImage.Width, (int)FrameServerImage.Height, 96, CanvasAlphaMode.Premultiplied/*DisplayInformation.GetForCurrentView().LogicalDpi*/);//96); 
                }


                using (CanvasBitmap inputBitmap = CanvasBitmap.CreateFromSoftwareBitmap(canvasDevice, frameServerDest))
                using (CanvasDrawingSession ds = canvasImageSource.CreateDrawingSession(Colors.Black))
                {
                    sender.CopyFrameToVideoSurface(inputBitmap);
                    ds.DrawImage(inputBitmap);
                    ds.Flush();
                    FrameServerImage.Source = canvasImageSource;
                }

            }
            catch
            {
                frameServerDest?.Dispose();
                frameServerDest = null;
                canvasImageSource = null;
            }
        }


        private void RenderSubtitleFrame(MediaPlayer sender, bool drawSubs)
        {
            try
            {
                canvasDevice = CanvasDevice.GetSharedDevice();

                if (sender.PlaybackSession.PlaybackState == MediaPlaybackState.Paused) return;
                SubtitleTexture.Width = MediaPlayerPresenter.ActualWidth;
                SubtitleTexture.Height = MediaPlayerPresenter.ActualHeight;
                //if (!Window.Current.Visible || SubtitleTexture.Width == 0 || SubtitleTexture.Height == 0) return;
                SubtitleTexture.Source = subRenderer.RenderSubtitleToSurface(sender.Source as MediaPlaybackItem, (float)SubtitleTexture.Width, (float)SubtitleTexture.Height).BitmapImageSource;
            }
            catch (Exception ex)
            {
                subtitleDest?.Dispose();
                subtitleDest = null;
                subtitleImageSource = null;
            }
        }

        private void RenderSubtitlesToSurface(CanvasBitmap inputBitmap, CanvasDrawingSession ds, MediaPlayer player)
        {
            var sourceItem = (MediaPlaybackItem)player.Source;
            for (int i = 0; i < sourceItem.TimedMetadataTracks.Count; i++)
            {
                var track = sourceItem.TimedMetadataTracks[i];
                if (track.TimedMetadataKind == Windows.Media.Core.TimedMetadataKind.Subtitle &&
                    sourceItem.TimedMetadataTracks.GetPresentationMode((uint)i) == TimedMetadataTrackPresentationMode.PlatformPresented)
                {
                    var activeCues = track.ActiveCues;
                    for (int j = 0; j < activeCues.Count; j++)
                    {
                        var cue = (TimedTextCue)activeCues[j];
                        var textBuilder = new StringBuilder();
                        for (int k = 0; k < cue.Lines.Count; k++)
                        {
                            var line = cue.Lines[k];
                            textBuilder.Append(line.Text);
                        }

                        CanvasTextLayout textLayout = new CanvasTextLayout(canvasDevice, textBuilder.ToString(), new CanvasTextFormat()
                        {
                            VerticalAlignment = CanvasVerticalAlignment.Bottom,
                            HorizontalAlignment = CanvasHorizontalAlignment.Center,

                        }, (float)inputBitmap.Size.Width, (float)inputBitmap.Size.Height);

                        ds.DrawTextLayout(textLayout, 0, 0, Colors.White);
                    }
                }
            }
        }
    }
}
