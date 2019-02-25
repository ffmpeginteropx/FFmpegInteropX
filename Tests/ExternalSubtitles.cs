using FFmpegInterop;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System;
using System.Threading.Tasks;
using Windows.Foundation.Collections;
using Windows.Media.Core;
using Windows.Storage;

namespace UnitTest.Windows
{
    [TestClass]
    public class ExternalSubtitles
    {
        [TestMethod]
        public async Task TestExternalSubtitlesAsync()
        {
            //var folder = await ApplicationData.Current.LocalFolder.GetFolderAsync("ExternalSubtitles");
            //var files = await folder.GetFilesAsync();
        }
    }
}
