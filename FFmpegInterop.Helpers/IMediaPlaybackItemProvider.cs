using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Media.Core;
using Windows.Media.Playback;

namespace FFmpegInterop.Helpers
{
    public interface IMediaPlaybackItemProvider
    {
        /// <summary>
        /// this will probably require additional parameters
        /// </summary>
        /// <returns></returns>
        IAsyncOperation<MediaPlaybackItem> GetPlaybackItem();
    }
}
