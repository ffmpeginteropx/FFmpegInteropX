using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Media.Playback;
using Windows.Foundation;
using System.Threading;

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

        MediaPlaybackItemRequestDeferal Deferal;




        public MediaPlaybackItemRequestArgs(MediaPlaybackItemRequestType type)
        {
            this.RequestType = type;


        }

        internal void SetDeferal(MediaPlaybackItemRequestDeferal deferal)
        {
            Deferal = deferal;
        }

        public MediaPlaybackItemRequestDeferal GetDeferal()
        {
            Deferal.Reset();
            return Deferal;
        }
    }

    public enum MediaPlaybackItemRequestType
    {
        CurrentItem,
        NextItem
    }
}