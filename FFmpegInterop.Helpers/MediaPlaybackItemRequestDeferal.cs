using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using Windows.Foundation;

namespace FFmpegInterop.Helpers
{
    public sealed class MediaPlaybackItemRequestDeferal
    {
        ManualResetEvent _event = new ManualResetEvent(true);

        public void Complete()
        {
            _event.Set();
        }

        public void WaitOnDeferal()
        {
            _event.WaitOne();
        }

        internal void Reset()
        {
            _event.Reset();
        }

        internal static async Task RunOnDeferal(Action<MediaPlaybackItemRequestArgs> action, MediaPlaybackItemRequestArgs param)
        {
            MediaPlaybackItemRequestDeferal deferal = new MediaPlaybackItemRequestDeferal();
            param.SetDeferal(deferal);
            var runner = Task.Run(() => { action(param); });
            var watchDog = Task.Run(() =>
            {
                while (!runner.IsCompleted)
                {
                    deferal.WaitOnDeferal();
                }
            });
            
            await Task.WhenAll(runner, watchDog);
        }
    }
}
