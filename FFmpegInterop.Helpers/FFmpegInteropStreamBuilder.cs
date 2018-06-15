using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Media.Playback;
using Windows.Storage.Streams;

namespace FFmpegInterop.Helpers
{
    public sealed class FFmpegInteropStreamBuilder : IFfmpegInteropBuilder
    {
        IRandomAccessStream sourceStream;
        FFmpegInteropConfig configuration;
        FFmpegInteropMSS currentMss;

        public FFmpegInteropConfig Configuration { get; }

        public TimeSpan? StartTime { get; }

        public TimeSpan? MediaDuration { get; }

        public FFmpegInteropStreamBuilder(IRandomAccessStream stream, FFmpegInteropConfig config, TimeSpan? start, TimeSpan? duration)
        {
            sourceStream = stream;
            configuration = config;
            StartTime = start;
            MediaDuration = duration;
        }

        public void Dispose()
        {
            sourceStream.Dispose();
            currentMss.Dispose();
        }

        public void Reset()
        {
            sourceStream.Seek(0);
            currentMss.Dispose();
        }

        public IAsyncOperation<MediaPlaybackItem> OpenFFmpegAsync()
        {
            Reset();
            return Task.Run<MediaPlaybackItem>(async () =>
            {
                if (currentMss == null)
                {
                    currentMss = await FFmpegInteropMSS.CreateFromStreamAsync(sourceStream);
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
