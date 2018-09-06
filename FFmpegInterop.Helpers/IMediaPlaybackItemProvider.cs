using Windows.Foundation;

namespace FFmpegInterop.Helpers
{
    public interface IMediaPlaybackItemProvider
    {
        /// <summary>
        /// This method must return a ffmpegInteropMSS object with an initialized media playback item
        /// </summary>
        /// <returns></returns>
        IAsyncOperation<FFmpegInteropMSS> GetNextItemAsync();


    }
}
