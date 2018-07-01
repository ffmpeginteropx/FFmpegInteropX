using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using Windows.Foundation;

namespace FFmpegInterop.Helpers
{
    public sealed class MediaPlaybackItemRequestOperation
    {
        internal TaskCompletionSource<bool> m_tcsDeferalCompleted
        {
            get;
            private set;
        }
        public MediaPlaybackItemRequestArgs Args { get; private set; }

        Deferral m_deferral;

        public Deferral GetDeferal()
        {
            return m_deferral;
        }

        private Action<MediaPlaybackItemRequestArgs> m_playbackItemRequest;


        internal MediaPlaybackItemRequestOperation(Action<MediaPlaybackItemRequestArgs> playbackItemRequest, MediaPlaybackItemRequestArgs args)
        {
            m_deferral = new Deferral(() =>
            {
                m_tcsDeferalCompleted.SetResult(true);
            });

            m_playbackItemRequest = playbackItemRequest;
            Args = args;
        }



        internal Task<bool> Run()
        {
            m_playbackItemRequest(Args);
            return m_tcsDeferalCompleted.Task;
        }

    }
}
