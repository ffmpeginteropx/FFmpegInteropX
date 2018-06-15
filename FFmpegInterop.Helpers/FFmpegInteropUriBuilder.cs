using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Media.Playback;

namespace FFmpegInterop.Helpers
{
    public sealed class FFmpegInteropUriBuilder : IFfmpegInteropBuilder
    {
        string Uri;
        FFmpegInteropMSS currentMss;

        public FFmpegInteropConfig Configuration { get; }

        public TimeSpan? StartTime { get; }

        public TimeSpan? MediaDuration { get; }

        public FFmpegInteropUriBuilder(string uri, FFmpegInteropConfig config, TimeSpan? start, TimeSpan? duration)
        {
            Uri = uri;
            Configuration = config;
            StartTime = start;
            MediaDuration = duration;
        }

        public void Reset()
        {
            currentMss.Dispose();
        }

        public void Dispose()
        {
            Uri = null;
            Reset();
        }

        public IAsyncOperation<MediaPlaybackItem> OpenFFmpegAsync()
        {
            return Task.Run<MediaPlaybackItem>(async () =>
            {
                if (currentMss == null)
                {
                    currentMss = await FFmpegInteropMSS.CreateFromUriAsync(Uri);
                }

                if (StartTime == null)
                    return currentMss.CreateMediaPlaybackItem();
                else
                {
                    if (MediaDuration == null)
                    {
                        return currentMss.CreateMediaPlaybackItem(StartTime.Value);
                    }
                    else
                    {
                        return currentMss.CreateMediaPlaybackItem(StartTime.Value, MediaDuration.Value);
                    }
                }
            }).AsAsyncOperation();
        }
    }
}
