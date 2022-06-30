#pragma once

namespace FFmpegInteropX
{
    namespace WFM = Windows::Foundation::Metadata;

    public enum class HardwareDecoderStatus
    {
        Unknown,
        Available,
        NotAvailable
    };

    public enum class DecoderEngine
    {
        SystemDecoder,
        FFmpegSoftwareDecoder,
        FFmpegD3D11HardwareDecoder
    };

    public enum class VideoDecoderMode
    {
        ///<summary>Use FFmpeg D3D11 hardware acceleration, if available. Otherwise, use FFmpeg software decoder</summary>
        ///<remarks>The VideoStreamInfo will show the actual decoder only after playback has started.</remarks>
        Automatic,
        ///<summary>Automatically detect hardware acceleration capabilities and use system decoders for those formats.</summary>
        ///<remarks>For some formats, installation of video codecs is required for this to work. Check CodecChecker.CodecRequired event.</remarks>
        AutomaticSystemDecoder,
        ///<summary>Force use of system decoder.</summary>
        ForceSystemDecoder,
        ///<summary>Force use of ffmpeg software decoder.</summary>
        ForceFFmpegSoftwareDecoder
    };

    ///<summary>This flags enumeration describes the content or intention of a stream.</summary>
    [Platform::Metadata::Flags]
    public enum class StreamDisposition : unsigned int
    {
        ///<summary>Unknown disposition.</summary>
        Unknown = 0,
        ///<summary>The stream should be chosen by default among other streams of the same type, unless the user has explicitly specified otherwise.</summary>
        Default = AV_DISPOSITION_DEFAULT,
        ///<summary>The stream is not in original language.</summary>
        Dub = AV_DISPOSITION_DUB,
        ///<summary>The stream is in original language.</summary>
        Original = AV_DISPOSITION_ORIGINAL,
        ///<summary>The stream is a commentary track.</summary>
        Comment = AV_DISPOSITION_COMMENT,
        ///<summary>The stream contains song lyrics.</summary>
        Lyrics = AV_DISPOSITION_LYRICS,
        ///<summary>The stream contains karaoke audio.</summary>
        Karaoke = AV_DISPOSITION_KARAOKE,
        ///<summary>Track should be used during playback by default.</summary>
        ///<remarks>Useful for subtitle track that should be displayed even when user did not explicitly ask for subitles. </remarks>
        Forced = AV_DISPOSITION_FORCED,
        ///<summary>The stream is intended for hearing impaired audiences.</summary>
        HearingImpaired = AV_DISPOSITION_HEARING_IMPAIRED,
        ///<summary>The stream is intended for visually impaired audiences.</summary>
        VisualImpaired = AV_DISPOSITION_VISUAL_IMPAIRED,
        ///<summary>The audio stream contains music and sound effects without voice.</summary>
        CleanEffects = AV_DISPOSITION_CLEAN_EFFECTS,
        ///<summary>The stream is stored in the file as an attached picture/"cover art" (e.g. APIC frame in ID3v2)</summary>
        AttachedPic = AV_DISPOSITION_ATTACHED_PIC,
        ///<summary>The stream is sparse, and contains thumbnail images, often corresponding to chapter markers.</summary>
        ///<remarks>Only ever used with AV_DISPOSITION_ATTACHED_PIC.</remarks>
        TimesThumbnails = AV_DISPOSITION_TIMED_THUMBNAILS,
        ///<summary>The subtitle stream contains captions, providing a transcription and possibly a translation of audio.</summary>
        ///<remarks>Typically intended for hearing-impaired audiences.</remarks>
        Captions = AV_DISPOSITION_CAPTIONS,
        ///<summary>The subtitle stream contains a textual description of the video content.</summary>
        ///<remarks>Typically intended for visually-impaired audiences or for the cases where the video cannot be seen.</remarks>
        Descriptions = AV_DISPOSITION_DESCRIPTIONS,
        ///<summary>The subtitle stream contains time-aligned metadata that is not intended to be directly presented to the user.</summary>
        Metadata = AV_DISPOSITION_METADATA,
        ///<summary>The audio stream is intended to be mixed with another stream before presentation.</summary>
        Dependent = AV_DISPOSITION_DEPENDENT,
        ///<summary>The video stream contains still images.</summary>
        StillImage = AV_DISPOSITION_STILL_IMAGE
    };
}