# FFmpegInteropX library for Windows

#### This project is licensed under the [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0)

## Welcome to FFmpegInteropX

FFmpegInteropX is an open-source project that aims to provide an easy way to use **FFmpeg** as a decoder library in **Windows 10/11 UWP Apps**. This allows you to decode a lot of formats that are not natively supported on Windows 10/11. Please note that only decoding is supported currently, we provide no encoding or transcoding support.

FFmpegInteropX is a much **improved fork** of the original [Microsoft project](git://github.com/Microsoft/FFmpegInterop).

#### Latest Releases:
- [FFmpegInteropX](https://www.nuget.org/packages/FFmpegInteropX):
  - 1.0.0
    - Renamed namespaces and classes
    - Removed deprecated APIs and code cleanup
  - 0.9.4
    - Support for HDR video!
  - 0.9.3
    - Support for AV1 hardware and software decoding
    - Dynamic detection of AV1 hardware decoding capabilities
- [FFmpegInteropX.FFmpegUWP](https://www.nuget.org/packages/FFmpegInteropX.FFmpegUWP): 
  - 5.1.100
    - FFmpeg 5.1.1 build for UWP platform
    - Includes unofficial "init_threads" option to speedup DASH stream loading
  - 5.0.0
    - FFmpeg 5.0.0 build for UWP platform
  - 4.4.100
    - FFmpeg 4.4.1 build for UWP platform
    - Added AV1 hardware decoder
    - Added dav1d library for AV1 software decoding
    - Added openssl 3.0.1 for secure streaming support (e.g. https, rtmps)
    - Build system improvements:
      - Added dockerfile for building inside container (experimental)
      - Automatic download and installation of all dependencies (both dockerfile and local build)
      - Support for building all target platforms in parallel

### Some of the important improvements, compared to original version:

- NuGet packages!!
- Multiple video and audio stream support
- Support for HDR video
- Subtitle support, including external subtitle files
- Native D3D11 hardware decoding for all major formats:
  - H264, HEVC, AV1, VC1, VP8, VP9, WMV3, MPEG2
- FFmpeg video and audio effects (special thanks to [mcosmin222](https://github.com/mcosmin222)!)
- Super fast GPU-based video postprocessing effects
- Major performance improvements (zero-copy data handling in all important areas)
- Frame grabber support
- Fast Seeking support (seek to keyframes)
- Stream information retrieval (name, language, format, etc)
- Chapter information support
- Stereo downmix option
- Improved support for image file formats
- API improvements
- Include zlib and bzlib libraries into ffmpeg for full MKV subtitle support
- Include iconv for character encoding conversion
- Include libxml2 for DASH streaming support
- Include openssl for secure streaming support (e.g. https, rtmps)
- Lots of bug fixes


## How to work with FFmpegInteropX

We have switched from manual builds to providing NuGet packages. There are two packages: 

- [**FFmpegInteropX**](https://www.nuget.org/packages/FFmpegInteropX)
  - The library itself, referenced by app project files
  - Has a dependency on FFmpegInteropX.FFmpegUWP, which contains the actual FFmpeg build

- [**FFmpegInteropX.FFmpegUWP**](https://www.nuget.org/packages/FFmpegInteropX.FFmpegUWP)
  - Our official FFmpeg build for the UWP platform
  - Customized and tested for use with FFmpegInteropX
  - Includes FFFmpeg dll files, libs, includes and license files
  - Two purposes:
    - Provide runtime dependencies (dlls) for apps
    - Provide build dependencies for our library

The easiest way to work with FFmpegInteropX is to add both NuGet packages to your app. This allows full usage of all features, without checking out the repo or installing build tools.

**Advanced users and library developers:** If you want to be able to debug into FFmpegInteropX right from your app, or to work on the library, you need to clone this repository. Instead of adding the FFmpegInteropX NuGet package to your app, you can directly add the `Source\FFmpegInteropX.vcxproj` project file to your app solution (it does not matter where the FFmpegInteropX folder is located). Then in your main app project, add a reference to the FFmpegInteropX project. Now you have all the sources directly in your app solution and can debug and enhance the lib.

**Full blown:** If needed, you can even supply your own custom FFmpeg build to replace our FFmpegInteropX.FFmpegUWP NuGet package.

Check out the [build instructions](README-BUILD.md) if you want to manually build FFmpgeInteropX or FFmpeg itself.

## Using the FFmpegInteropX libraray

Using the **FFmpegMediaSource** object is fairly straightforward and can be observed from the sample applications provided.

1. Get an `IRandomAccessStream` for the media you want to playback.
2. Create a new `FFmpegMediaSource` object using `FFmpegMediaSource.CreateFromStreamAsync()` passing it the stream and optionally a config class instance.
3. Get the MediaPlaybackItem from the Interop object by invoking `CreateMediaPlaybackItem()`
4. Assign the MediaPlaybackItem to your MediaPlayer or MediaElement for playback.

**Important:** Store the FFmpegMediaSource instance e.g. in a local field, as long as playback is running. If the object is collected by the GC during playback, playback will stop with an error.

You can use `FFmpegMediaSource.GetMediaStreamSource()` to get the MediaStreamSource like in the original version of the library. But when using MediaStreamSource, you won't get subtitles. Subtitle support requires using the MediaPlaybackItem!

## Using the FFmpegInteropX for streaming

Use `FFmpegMediaSource.CreateFromUriAsync()` to create a MediaStreamSource on a streaming source (video stream, live stream, shoutcast audio stream, ...).

There are a lot of FFmpeg parameters that can be set, especially for streaming scenarios.
You most definitely want to enable automatic reconnection on errors:

```
config.FFmpegOptions = new Windows.Foundation.Collections.PropertySet {
    { "reconnect", 1 },
    { "reconnect_streamed", 1 },
    { "reconnect_on_network_error", 1 },
}
```

You should also consider setting timeouts, but be careful since each protocol might have different timeout parameters and meanings.

## Stream Buffering

If you do web streaming, you should try the new read-ahead buffer feature of FFmpegInteropX, available in the latest prerelease packages. It will read ahead and buffer a configurable amount of stream data to allow uninterrupted playback. This even allows fast forward seeking (stepping) without stream reconnection, if the target position is buffered.

```
config.ReadAheadBufferEnabled = true;

// Optionally, configure buffer size (max duration and byte size)
config.ReadAheadBufferDuration = TimeSpan.FromSeconds(30);
config.ReadAheadBufferEnabled = 50*1024*1024;
```

Check FFmpeg documentation on [Protocols](https://ffmpeg.org/ffmpeg-protocols.html) and [Formats](https://ffmpeg.org/ffmpeg-formats.html).

## Frame Grabber

You can use the **FrameGrabber** class to extract video frames at specific positions of a video file or video stream, optionally using fast seeking to keyframes and optionally downscaling the video resolution. Extracted frames contain raw image data and can be stored to different formats (JPEG, BMP, PNG).

Simplified example calls:

```
var fileStream = await file.OpenRead();
var frameGrabber = await FrameGrabber.CreateFromStreamAsync(fileStream);
var frame = await frameGrabber.ExtractVideoFrameAsync(TimeSpan.FromSeconds(10));
await frame.EncodeAsJpegAsync(outputStream);
```

## Subtitle Support

FFmpegInterop will automatically load and use all embedded subtitles, supporting all formats through ffmpeg. You have to use the MediaPlaybackItem returned from the FFmpegMediaSource object. Then subtitles can be selected from MediaElement's transport controls. 

You can also add external subtitle files by using `FFmpegMediaSource.AddExternalSubtitleAsync()`, even during playback. See the sample apps for reference. All ffmpeg subtitle formats are supported as external files, except for the two-file "sub/idx" (DVD) format. 

Some external text subtitle files are stored with ANSI encoding instead of UTF8 (which is required by ffmpeg). FFmpegInterop can do an automatic conversion to UTF8. This is enabled by default in the config class and will use the system's active codepage by default. You can change the behavior by changing `AutoCorrectAnsiSubtitles` and `AnsiSubtitleEncoding` parameters in the config class. Codepage 0 is the system's active codepage.

Note: If your app uses multiple windows using CoreApplication.CreateNewView(), then you must create the FFmpegMediaSource object on the thread of the window where the video is to be shown. Otherwise, subtitles will flicker.

#### Version History:
- [FFmpegInteropX](https://www.nuget.org/packages/FFmpegInteropX): 0.9.2
  - Native D3D11 hardware acceleration!!
  - FFmpeg video filters
  - Fast seeking to keyframes
  - Stereo downmix option
  - Improved support for image file formats
- [FFmpegInteropX.FFmpegUWP](https://www.nuget.org/packages/FFmpegInteropX.FFmpegUWP): 4.3.100
  - FFmpeg 4.3.1 build for UWP platform
  - D3D11 hardware acceleration enabled

## Credits / major contributors

- [lukasf](https://github.com/lukasf)
- [mcosmin222](https://github.com/mcosmin222)
- [MouriNaruto](https://github.com/MouriNaruto)
- [JunielKatarn](https://github.com/JunielKatarn)

Many more helped with development, bug reports and suggestions. Thank you all!

Thank you also to the Microsoft team who developed the original library!

## Your feedback is appreciated!

Feel free to open issues, pull requests, or join discussions.
