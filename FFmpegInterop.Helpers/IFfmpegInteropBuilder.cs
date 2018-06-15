using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using FFmpegInterop;
using Windows.Media.Playback;

namespace FFmpegInterop.Helpers
{
    public interface IFfmpegInteropBuilder : IDisposable
    {
        IAsyncOperation<MediaPlaybackItem> OpenFFmpegAsync();

        FFmpegInteropConfig Configuration
        {
            get;
        }

        TimeSpan? StartTime
        {
            get;
        }

        TimeSpan? MediaDuration
        {
            get;
        }

        void Reset();

    }
}
