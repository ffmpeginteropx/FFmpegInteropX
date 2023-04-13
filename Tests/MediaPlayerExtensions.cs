using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Media.Core;
using Windows.Media.Playback;

namespace FFmpegInteropX.UnitTests
{
    public static class MediaPlayerExtensions
    {
        public static async Task SeekAsync(this MediaPlayer CurrentPlayer, TimeSpan position)
        {
            TaskCompletionSource<bool> SeekCompletedSignal = new TaskCompletionSource<bool>();

            TypedEventHandler<MediaPlaybackSession, object> seekHandler = (s, e) =>
            {
                SeekCompletedSignal.SetResult(true);
            };

            CurrentPlayer.PlaybackSession.SeekCompleted += seekHandler;
            CurrentPlayer.PlaybackSession.Position = position;
            await SeekCompletedSignal.Task;

            CurrentPlayer.PlaybackSession.SeekCompleted -= seekHandler;
        }

        public static async Task<bool> SetMediaAsync(this MediaPlayer CurrentPlayer, IMediaPlaybackSource source)
        {
            TaskCompletionSource<bool> MediaOpenedOrFailedSignal = new TaskCompletionSource<bool>();

            TypedEventHandler<MediaPlayer, object> mediaOpenedHandler = (s, e) =>
            {
                MediaOpenedOrFailedSignal.SetResult(true);
            };

            TypedEventHandler<MediaPlayer, MediaPlayerFailedEventArgs> mediaFailedHandler = (s, e) =>
            {
                MediaOpenedOrFailedSignal.SetResult(false);
            };

            CurrentPlayer.MediaOpened += mediaOpenedHandler;
            CurrentPlayer.MediaFailed += mediaFailedHandler;

            CurrentPlayer.Source = source;
            await MediaOpenedOrFailedSignal.Task;

            CurrentPlayer.MediaOpened -= mediaOpenedHandler;
            CurrentPlayer.MediaFailed -= mediaFailedHandler;

            return await MediaOpenedOrFailedSignal.Task;
        }
    }
}
