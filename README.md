# FFmpegInteropX library for Windows

#### This project is licensed under the [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0)

## Welcome to FFmpegInteropX

FFmpegInteropX is an open-source project that aims to provide an easy way to use **FFmpeg** in **Windows 10 UWP Apps**. This allows you to decode a lot of formats that are not natively supported on Windows 10.

FFmpegInteropX is a much **improved fork** of the original [Microsoft project](git://github.com/Microsoft/FFmpegInterop).

#### Latest Releases:
- [FFmpegInteropX](https://www.nuget.org/packages/FFmpegInteropX): 0.9.2
  - Native D3D11 hardware acceleration!!
  - FFmpeg video filters
  - Fast seeking to keyframes
  - Stereo downmix option
  - Improved support for image file formats
- [FFmpegInteropX.FFmpegUWP](https://www.nuget.org/packages/FFmpegInteropX.FFmpegUWP): 4.3.100
  - FFmpeg 4.3.1 build for UWP platform
  - D3D11 hardware acceleration enabled

### Some of the important improvements, compared to original version:

- NuGet packages!!
- Multiple video and audio stream support
- Subtitle support, including external subtitle files
- Native D3D11 hardware decoding for all major formats:
  - H264, HEVC, VC1, VP8, VP9, WMV3, MPEG2
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

**Advanced users and library developers:** If you want to be able to debug into FFmpegInteropX right from your app, or to work on the library, you need to clone this repository. Instead of adding the FFmpegInteropX NuGet package to your app, you can directly add the `FFmpegInterop\FFmpegInterop.vcxproj` project file to your app solution (it does not matter where the FFmpegInteropX folder is located). Then in your main app project, add a reference to the FFmpegInterop project. Now you have all the sources directly in your app solution and can debug and enhance the lib.

**Full blown:** If needed, you can even supply your own custom FFmpeg build to replace our FFmpegInteropX.FFmpegUWP NuGet package.

Check out the [build instructions](README-BUILD.md) if you want to manually build FFmpgeInteropX or FFmpeg itself.

## Using the FFmpegInteropX libraray

Using the **FFmpegInteropMSS** object is fairly straightforward and can be observed from the sample applications provided.

1. Get an `IRandomAccessStream` for the media you want to playback.
2. Create a new `FFmpegInteropMSS` object using `FFmpegInteropMSS.CreateFromStreamAsync()` passing it the stream and optionally a config class instance.
3. Get the MediaPlaybackItem from the Interop object by invoking `CreateMediaPlaybackItem()`
4. Assign the MediaPlaybackItem to your MediaPlayer or MediaElement for playback.

**Important:** Store the FFmpegInteropMSS instance e.g. in a local field, as long as playback is running. If the object is collected by the GC during playback, playback will stop with an error.

Use `FFmepgInteropMSS.CreateFromUriAsync()` to create a MediaStreamSource on a streaming source (shoutcast for example).

You can use `FFmpegInteropMSS.GetMediaStreamSource()` to get the MediaStreamSource like in the original version of the library. But when using MediaStreamSource, you won't get subtitles. Subtitle support requires using the MediaPlaybackItem!

You can add a call to `FFmpegVersionInfo.CheckRecommendedVersion()` in your app startup code. This will raise an exception if you are using the lib with a version lower than the recommended version. This can help remind you to update ffmpeg after you updated FFmpegInterop.

Call `FrameGrabber.CreateFromStreamAsync()` to grab one or more frames from a video file.

## Subtitle Support

FFmpegInterop will automatically load and use all embedded subtitles, supporting all formats through ffmpeg. You have to use the MediaPlaybackItem returned from the MSS object. Then subtitles can be selected from MediaElement's transport controls. 

You can also add external subtitle files by using `FFmpegInteropMSS.AddExternalSubtitleAsync()`, even during playback. See the sample apps for reference. All ffmpeg subtitle formats are supported as external files, except for the two-file "sub/idx" (DVD) format. 

Some external text subtitle files are stored with ANSI encoding instead of UTF8 (which is required by ffmpeg). FFmpegInterop can do an automatic conversion to UTF8. This is enabled by default in the config class and will use the system's active codepage by default. You can change the behavior by changing `AutoCorrectAnsiSubtitles` and `AnsiSubtitleEncoding` parameters in the config class. Codepage 0 is the system's active codepage.

Note: If your app uses multiple windows using CoreApplication.CreateNewView(), then you must create the FFmpegInteropMSS object on the thread of the window where the video is to be shown. Otherwise, subtitles will flicker.

## FFmpegUniversal

A branch exists which makes FFmpegInterop compatible with FFmpegUniversal builds. It is a project which merges the individual ffmpeg dlls into one single file. This can make deployment easier. Check [FFmpegUniversal](https://github.com/M2Team/FFmpegUniversal) for more details.

## Credits / major contributors

- [lukasf](https://github.com/lukasf)
- [mcosmin222](https://github.com/mcosmin222)
- [MouriNaruto](https://github.com/MouriNaruto)
- [JunielKatarn](https://github.com/JunielKatarn)

Many more helped with development, bug reports and suggestions. Thank you all!

Thank you also to the Microsoft team who developed the original library!

## Your feedback is appreciated!

Feel free to open issues, pull requests, or join discussions.