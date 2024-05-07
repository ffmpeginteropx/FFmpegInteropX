using FFmpegInteropX;

namespace MediaPlayerCS
{
    /// <summary>
    /// Example usage for IMediaStreamFilter
    /// Set it in configuration before calling CreateFrom* methods
    /// Config.General.MediaStreamFilter = new StreamFilter();
    ///  
    /// </summary>
    class StreamFilter : IMediaStreamFilter
    {
        public bool ShouldAddStream(IStreamInfo streamInfo)
        {
            return streamInfo.StreamIndex != 1;
        }
    }
}
