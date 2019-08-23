# FFmpegInteropX library for Windows

#### This project is licensed under the [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0)

## Welcome to FFmpegInteropX

FFmpegInteropX is an open-source project that aims to provide an easy way to use **FFmpeg** in **Windows 10 UWP Apps**. This allows you to decode a lot of formats that are not natively supported on Windows 10.

FFmpegInteropX is a much **improved fork** of the original [Microsoft project](git://github.com/Microsoft/FFmpegInterop).

**Some of the important improvements:**

- Multiple audio stream support
- Subtitle support, including external subtitle files
- Audio effects (special thanks to [mcosmin222](https://github.com/mcosmin222)!)
- Stream information retrieval (name, language, format, etc)
- Passthrough of the following video formats for hardware decoding
  - HEVC
  - VC1 (Used in VC1 Advanced Profile)
  - WMV3 (Used in WMV9 and VC1 Simple and Main Profile)
  - MPEG2 (Requires "MPEG-2 Video Extensions" from Windows Store)
  - VP9 (Requires "VP9 Video Extensions" from Windows Store)
- Major performance improvements (zero-copy data handling in all important areas)
- Frame grabber support
- API improvements
- Include zlib and bzlib libraries into ffmpeg for full MKV subtitle support
- Include iconv for character encoding conversion
- Lots of bug fixes

**Other changes:**
- Support for Windows 8.x and Windows Phone 8.x has been dropped
- Visual Studio 2015 support has been dropped

**Prerequisites:**

Either Visual Studio 2017 or Visual Studio 2019 is required.

- Visual Studio 2017 (15.9.x or higher):
  - Select Universal Windows Platform development workload in Installer
  - Select additional components from Installer:
    - Universal Windows Platform tools
    - VC++ 2017 version 15.9 v14.16 latest v141 tools
    - Win 10 SDK (10.0.15063.0) for uwp: c#, vb, js
    - Win 10 SDK (10.0.15063.0) for uwp: c++
    - Visual C++ compilers and libraries for ARM64
    - Visual C++ compilers and libraries for ARM
    - C++ UWP tools for ARM64
    - C++ runtime for uwp

- Visual Studio 2019:
  - Select Universal Windows Platform development workload in Installer
  - Manually install Windows 10 SDK 10.0.15063.0 from SDK archive:
    https://developer.microsoft.com/en-us/windows/downloads/sdk-archive


## FFmpeg Version

Recommended: **FFmpeg 4.1.1**

Minimum: **FFmpeg 4.0**

An exception will be thrown if FFmpegInterop is used with anything lower than the minimum version. The recommended version has been tested and is what we currently recommend to use with FFmpegInteropX.

A legacy branch exists which tagets **FFmpeg 3.4.2**.

**Hint:** To update the ffmpeg submodule to the recommended version (after pulling latest FFmpegInterop), use the following command in git bash from FFmpegInterop folder: `git submodule update`. Then rebuild ffmpeg and FFmpegInterop.

## Getting the sources

FFmpegInteropX uses the following git submodules:

- ffmpeg
- Libs\bzlib2
- Libs\iconv
- Libs\zlib

Please use clone recursive, to get the exact verion of the libs that is required for use with FFmpegInteropX.

	git clone --recursive https://github.com/ffmpeginteropx/FFmpegInteropX.git

If you forgot to clone recursive, or if one of the library folders is empty, use these commands from FFmpegInteropX folder:

	git submodule init
    git submodule update

Please do not use later versions of FFmpeg (e.g. master branch) with FFmpegInteropX. This could lead to various problems, ranging from build issues to runtime issues.

Your `FFmpegInteropX` folder should look as follows

	FFmpegInteropX\
	    ffmpeg\                - ffmpeg source code from the latest release in git://github.com/FFmpeg/FFmpeg.git
	    FFmpegInterop\         - FFmpegInterop WinRT component
	    Libs\bzip2\            - bzip2 (bzliib) compression library
	    Libs\iconv\            - iconv library for character encoding conversion
	    Libs\zlib\             - zlib compression library
	    Samples\               - Sample Media Player applications in C++ and C#
	    Tests\                 - Unit tests for FFmpegInterop
	    BuildFFmpeg_VS2017.bat - FFmpeg build file for Visual Studio 2017 and higher
	    FFmpegConfig.sh        - Internal script that contains FFmpeg configure options
	    FFmpegInterop.sln      - Microsoft Visual Studio solution file for Windows 10 apps development
	    LICENSE
	    README.md

## Installing ffmpeg build tools

Now that you have the FFmpeg source code, you can follow the instructions on how to [build FFmpeg for WinRT](https://trac.ffmpeg.org/wiki/CompilationGuide/WinRT) apps. *Follow the setup instruction very carefully to avoid build issues!! Be very careful not to miss a single step. If you have problems building ffmpeg, go through these steps again, since chances are high that you missed some detail.*

## Building ffmpeg with Visual Studio 2017 / 2019

After installing the ffmpeg build tools, you can invoke `BuildFFmpeg_VS2017.bat` from a normal cmd prompt. It builds all Windows 10 versions of ffmpeg (x86, x64, ARM and ARM64). 

Note: You need Visual Studio 2017 15.9.0 or higher to build the ARM64 version of ffmpeg!

## Building the FFmpegInterop library

After building ffmpeg with the steps above, you should find the ffmpeg libraries in the `ffmpeg/Build/<platform\>/<architecture\>` folders.

Now you can build the FFmpegInterop library. 

Simply open the Visual Studio solution file `FFmpegInterop.sln`, set one of the MediaPlayer[CS/CPP/JS] sample projects as StartUp project, and run. FFmpegInterop should build cleanly giving you the interop object as well as the selected sample MediaPlayer (C++, C# or JS) that show how to connect the MediaStreamSource to a MediaElement or Video tag for playback.

### Using the FFmpegInteropMSS object

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

### Subtitle Support

FFmpegInterop will automatically load and use all embedded subtitles, supporting all formats through ffmpeg. You have to use the MediaPlaybackItem returned from the MSS object. Then subtitles can be selected from MediaElement's transport controls. 

You can also add external subtitle files by using `FFmpegInteropMSS.AddExternalSubtitleAsync()`, even during playback. See the sample apps for reference. All ffmpeg subtitle formats are supported as external files, except for the two-file "sub/idx" (DVD) format. 

Some external text subtitle files are stored with ANSI encoding instead of UTF8 (which is required by ffmpeg). FFmpegInterop can do an automatic conversion to UTF8. This is enabled by default in the config class and will use the system's active codepage by default. You can change the behavior by changing `AutoCorrectAnsiSubtitles` and `AnsiSubtitleEncoding` parameters in the config class. Codepage 0 is the system's active codepage.

Note: If your app uses multiple windows using CoreApplication.CreateNewView(), then you must create the FFmpegInteropMSS object on the thread of the window where the video is to be shown. Otherwise, subtitles will flicker.

## Integrating FFmpegInterop into your app solution

If you want to integrate FFmpegInterop into your app, you can just add the project file (`FFmpegInterop\FFmpegInterop.vcxproj`) to your app solution as an existing project and add a reference from your main app project to FFmpegInterop. The FFmpegInterop project does not have to be in your app's solution folder. 

Additionally, your app must reference the ffmpeg dll files for the platform you are building. Best is to manually edit your app's project file. This allows you to refer the dlls built for the current platform using $BuildPlatform parameter.

For a C# project, you can do it like this:

```
  <ItemGroup>
    <Content Include="$(SolutionDir)..\FFmpegInterop\ffmpeg\Build\Windows10\$(PlatformTarget)\bin\*.dll" />
  </ItemGroup>
```

This assumes that the FFmpegInterop folder is located next to your solution folder and the ffmpeg build output folder contains exclusively the latest ffmpeg dlls. Paths might be different on your folder setup. For CPP and JS this can be done similarly, check the samples for reference.

## FFmpegUniversal

A branch exists which makes FFmpegInterop compatible with FFmpegUniversal builds. It is a project which merges the individual ffmpeg dlls into one single file. This can make deployment easier. Check [FFmpegUniversal](https://github.com/M2Team/FFmpegUniversal) for more details.

## Credits / contributors

- [lukasf](https://github.com/lukasf)
- [mcosmin222](https://github.com/mcosmin222)
- [MouriNaruto](https://github.com/MouriNaruto)

Thank you also to Microsoft and the team who developed the original library!

## Your feedback is appreciated!
