using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Media.Playback;
using Windows.Foundation;

namespace FFmpegInterop.Helpers
{
    public sealed class MediaPlaybackItemRequestArgs
    {
        public MediaPlaybackItem RequestItem
        {
            get;
            set;
        }

        public MediaPlaybackItemRequestType RequestType
        {
            get;
            private set;
        }

        public Deferral Deferal
        {
            get;
            private set;
        }

        public MediaPlaybackItemRequestArgs(MediaPlaybackItemRequestType type)
        {
            this.RequestType = type;
            this.Deferal = new Deferral(() =>
            {//no-op yet });
            });

        }

    }


    public enum MediaPlaybackItemRequestType
    {
        CurrentItem,
        NextItem
    }
}