# FFmpegInterop library for Windows

#### This project is licensed under the [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0)

## Welcome to FFmpegInterop-lukasf

FFmpegInterop is an open-source project that aims to provide an easy way to use **FFmpeg** in **Windows 10 UWP Apps**. This allows you to decode a lot of formats that are not natively supported on Windows 10.

FFmpegInterop-lukasf is a much **improved fork** of the original [Microsoft project](git://github.com/Microsoft/FFmpegInterop).

**Some of the important improvements:**

- Multiple audio stream support
- Subtitle support
- Audio effects (special thanks to [mcosmin222](https://github.com/mcosmin222)!)
- Stream information retrieval (name, language, format, etc)
- Passthrough of HEVC video for hardware decoding
- Major performance improvements (zero-copy data handling in all important areas)
- Frame grabber support
- API improvements
- Lots of bug fixes

**Other changes:**
- Support for Windows 8.x and Windows Phone 8.x has been dropped
- Visual Studio 2015 support has been dropped

## FFmpeg Version

Recommended: **FFmpeg 4.1**

Minimum: **FFmpeg 4.0**

An exception will be thrown if FFmpegInterop is used with anything lower than the minimum version. The recommended version has been tested and is what we currently recommend to use with FFmpegInterop-lukasf/master.

A legacy branch exists which tagets **FFmpeg 3.4.2**.

**Hint:** To update the ffmpeg submodule to the recommended version (after pulling latest FFmpegInterop), use the following command in git bash from FFmpegInterop folder: `git submodule update`. Then rebuild ffmpeg and FFmpegInterop.

## Getting the sources

Getting a compatible UWP build of FFmpeg is required for this to work.

You can simply use the embedded git submodule that points to the latest tested release of FFmpeg.

	git clone --recursive git://github.com/lukasf/FFmpegInterop.git

Alternatively, you can get the code for [FFmpeg on Github](http://github.com/FFmpeg) yourself by cloning [git://source.ffmpeg.org/ffmpeg.git](git://source.ffmpeg.org/ffmpeg.git) and replace existing default `ffmpeg` folder with it.

	git clone git://github.com/lukasf/FFmpegInterop.git
	cd FFmpegInterop
	git clone git://source.ffmpeg.org/ffmpeg.git

Your `FFmpegInterop` folder should look as follows

	FFmpegInterop\
	    ffmpeg\                - ffmpeg source code from the latest release in git://github.com/FFmpeg/FFmpeg.git
	    FFmpegInterop\         - FFmpegInterop WinRT component
	    Samples\               - Sample Media Player applications in C++ and C#
	    Tests\                 - Unit tests for FFmpegInterop
	    BuildFFmpeg_VS2015.bat - FFmpeg build file for Visual Studio 2015
	    BuildFFmpeg_VS2017.bat - FFmpeg build file for Visual Studio 2017
	    FFmpegConfig.sh        - Internal script that contains FFmpeg configure options
	    FFmpegInterop.sln      - Microsoft Visual Studio 2017 solution file for Windows 10 apps development
	    LICENSE
	    README.md

## Installing ffmpeg build tools

Now that you have the FFmpeg source code, you can follow the instructions on how to [build FFmpeg for WinRT](https://trac.ffmpeg.org/wiki/CompilationGuide/WinRT) apps. *Follow the setup instruction very carefully to avoid build issues!! Be very careful not to miss a single step. If you have problems building ffmpeg, go through these steps again, since chances are high that you missed some detail.*

## Building ffmpeg with Visual Studio 2017

After installing the ffmpeg build tools, you can invoke `BuildFFmpeg_VS2017.bat` from a normal cmd prompt. It builds all Windows 10 versions of ffmpeg (x86, x64, ARM and ARM64). 

Note: You need Visual Studio 2017 15.9.0 or higher to build the ARM64 version of ffmpeg!

Note: If you have Visual Studio 2015 installed as well, please try the Visual Studio 2015 build file (see below). Some people reported problems when having both installed and running the VS2017 file.

## Building ffmpeg with Visual Studio 2015

After installing the ffmpeg build tools, you can invoke the `BuildFFmpeg_VS2015.bat` script.

	BuildFFmpeg_VS2015.bat win10                     - Build for Windows 10 ARM, x64, and x86
	BuildFFmpeg_VS2015.bat phone8.1 ARM              - Build for Windows Phone 8.1 ARM only
	BuildFFmpeg_VS2015.bat win8.1 x86 x64            - Build for Windows 8.1 x86 and x64 only
	BuildFFmpeg_VS2015.bat phone8.1 win10 ARM        - Build for Windows 10 and Windows Phone 8.1 ARM only
	BuildFFmpeg_VS2015.bat win8.1 phone8.1 win10     - Build all architecture for all target platform

Alternatively, you can build the ffmpeg dlls manually using the instructions in the [ffmpeg compilation guide](https://trac.ffmpeg.org/wiki/CompilationGuide/WinRT).

## Building FFmpegInterop library

After building ffmpeg with the steps above, you should find the ffmpeg libraries in the `ffmpeg/Build/<platform\>/<architecture\>` folders.

Now you can build the FFmpegInterop library. 

Simply open the Visual Studio solution file `FFmpegInterop.sln`, set one of the MediaPlayer[CS/CPP/JS] sample projects as StartUp project, and run. FFmpegInterop should build cleanly giving you the interop object as well as the selected sample MediaPlayer (C++, C# or JS) that show how to connect the MediaStreamSource to a MediaElement or Video tag for playback.

### Using the FFmpegInterop object

Using the **FFmpegInteropMSS** object is fairly straightforward and can be observed from the sample applications provided.

1. Get an `IRandomAccessStream` for the media you want to playback.
2. Create a new `FFmpegInteropMSS` object using `FFmpegInteropMSS.CreateFromStreamAsync()` passing it the stream and optionally a config class instance.
3. Get the MediaPlaybackItem from the Interop object by invoking `CreateMediaPlaybackItem()`
4. Assign the MediaPlaybackItem to your MediaPlayer or MediaElement for playback.

Use `FFmepgInteropMSS.CreateFromUriAsync()` to create a MediaStreamSource on a streaming source (shoutcast for example).

You can use `FFmpegInteropMSS.GetMediaStreamSource()` to get the MediaStreamSource like in the original version of the library. But when using MediaStreamSource, you won't get subtitles. Subtitle support requires using the MediaPlaybackItem!

You can add a call to `FFmpegVersionInfo.CheckRecommendedVersion()` in your app startup code. This will raise an exception if you are using the lib with a version lower than the recommended version. This can help remind you to update ffmpeg after you updated FFmpegInterop.

Call `FrameGrabber.CreateFromStreamAsync()` to grab one or more frames from a video file.

## Integrating FFmpegInterop into your app solution

If you want to integrate FFmpegInterop into your app, you can just add the project file (`FFmpegInterop\Win10\FFmpegInterop\FFmpegInterop.vcxproj`) to your app solution as an existing project and add a reference from your main app project to FFmpegInterop. The FFmpegInterop project does not have to be in your app's solution folder. 

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
