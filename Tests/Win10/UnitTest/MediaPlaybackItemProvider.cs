using FFmpegInterop;
using FFmpegInterop.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Media.Playback;
using Windows.Storage;

namespace UnitTest
{
    internal class MediaPlaybackItemProvider : IMediaPlaybackItemProvider
    {
        int currentIndex = 0;
        List<string> files = new List<string>();

        public MediaPlaybackItemProvider()
        {
            files.Add("ms-appx:///tone.ogg");
            files.Add("ms-appx:///chirp.ogg");
            files.Add("ms-appx:///noise.ogg");
        }

        public IAsyncOperation<MediaPlaybackItem> GetPlaybackItem()
        {
            var returnValue = CreateMediaPlaybackItem(files[currentIndex]).AsAsyncOperation();
            currentIndex++;
            if (currentIndex == files.Count) currentIndex = 0;
            return returnValue;
        }


        internal static async Task<MediaPlaybackItem> CreateMediaPlaybackItem(string url)
        {
            var uri = new Uri(url);
            var file = await StorageFile.GetFileFromApplicationUriAsync(uri);
            var stream = await file.OpenAsync(FileAccessMode.Read);

            // CreateFFmpegInteropMSSFromUri should return null if uri is blank with default parameter
            FFmpegInteropMSS FFmpegMSS = await FFmpegInteropMSS.CreateFromStreamAsync(stream);
            return FFmpegMSS.CreateMediaPlaybackItem();
        }
    }
}
