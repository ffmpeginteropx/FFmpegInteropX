using FFmpegInteropX;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Media.Core;
using Windows.Media.Playback;

namespace MediaPlayerCS
{
    public class MediaPlaybackItemExtradata : IDisposable
    {
        public const string MediaSourceKey = "MediaPlaybackItemExtradata";
        private bool disposedValue;

        public FFmpegMediaSource MediaSource { get; private set; }

        public MediaPlaybackItemExtradata(FFmpegMediaSource mediaSource)
        {
            MediaSource = mediaSource;
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: dispose managed state (managed objects)
                }

                // TODO: free unmanaged resources (unmanaged objects) and override finalizer
                MediaSource.Dispose();
                // TODO: set large fields to null
                disposedValue = true;
            }
        }

        // TODO: override finalizer only if 'Dispose(bool disposing)' has code to free unmanaged resources
        ~MediaPlaybackItemExtradata()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

    public static class MediaPlaybackItemExtradataExtensions
    {
        public static void SetExtradata(this MediaPlaybackItem item, FFmpegMediaSource mediaSource)
        {
            item.Source.CustomProperties.Add(MediaPlaybackItemExtradata.MediaSourceKey, new MediaPlaybackItemExtradata(mediaSource));
        }

        public static MediaPlaybackItemExtradata GetExtradata(this MediaPlaybackItem item)
        {
            return (MediaPlaybackItemExtradata)item.Source.CustomProperties[MediaPlaybackItemExtradata.MediaSourceKey];
        }
    }
}
