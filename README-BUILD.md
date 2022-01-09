# FFmpegInteropX Build Instructions

**Prerequisites:**

Visual Studio 2019 is required to manually build FFmpegInteropX and FFmpeg. All other dependencies are installed automatically into the "Tools" subfolder during build. 

- Visual Studio 2019:
  - Select "Universal Windows Platform development" workload in Installer
  - Select "Desktop development with C++" workload in Installer
  - In VS Installer, check "Installation details" area on the right side, expand "Universal Windows Platform development", check "C++ (v142) Universal Windows Platform tools"
  - Visual C++ Redistributable for Visual Studio 2010 [x64](https://www.microsoft.com/en-us/download/details.aspx?id=14632) or [x86](https://www.microsoft.com/de-de/download/details.aspx?id=5555) (only needed for manual FFmpeg builds)
  - For AV1 HW decoding, you need the Windows 11 SDK 10.0.22000.0 (only available on Visual Studio 16.11.8 and higher). HW decoding will also run on Windows 10, the Windows 11 SDK is just required for building.

## Getting the sources

FFmpegInteropX uses the following git submodules:

- Libs\ffmpeg
- Libs\bzlib2
- Libs\iconv
- Libs\liblzma
- Libs\libxml2
- Libs\zlib

Please use clone recursive, to get the exact verion of the libs that is required for use with FFmpegInteropX.

	git clone --recursive https://github.com/ffmpeginteropx/FFmpegInteropX.git

If you forgot to clone recursive, or if one of the library folders is empty, use these commands from FFmpegInteropX folder:

	git submodule init
    git submodule update

Please do not use later versions of FFmpeg (e.g. master branch) with FFmpegInteropX. This could lead to various problems, ranging from build issues to runtime issues.

Your `FFmpegInteropX` folder should look as follows

	FFmpegInteropX\
	    FFmpegInterop\         - FFmpegInterop WinRT component
	    Libs\...               - Multiple Libraries required for building FFmpeg
	    Samples\               - Sample Media Player applications in C++ and C#
	    Tests\                 - Unit tests for FFmpegInterop
	    Build-FFmpeg.ps1       - FFmpeg build file for Visual Studio 2017 and higher
	    FFmpegConfig.sh        - Internal script that contains FFmpeg configure options
	    FFmpegInterop.sln      - Microsoft Visual Studio solution file for Windows 10 apps development
	    LICENSE
	    README.md


## Building ffmpeg with Visual Studio 2019

After installing the ffmpeg build tools, you run Build-FFmpeg.ps1 to build FFmpeg.

Run the build script from PowerShell:

`.\Build-FFmpeg.ps1`

Run the build script from CMD:

`PowerShell -NoProfile -ExecutionPolicy Bypass -Command ".\Build-FFmpeg.ps1"`

##### The build script has multiple parameters to select build toolset, SDK version and more. Here are some examples:

Build using a specific SDK version:

`.\Build-FFmpeg.ps1 -WindowsTargetPlatformVersion 10.0.17763.0`

Or from CMD:

`PowerShell -NoProfile -ExecutionPolicy Bypass -Command ".\Build-FFmpeg.ps1 -WindowsTargetPlatformVersion 10.0.17763.0"`


Build with PlatformToolset v142 instead of v141:

`.\Build-FFmpeg.ps1 -VcVersion 14.22`

(This requires MSVC v142 build tools "14.22" (exactly!) to be installed.)


Build using Visual Studio 2017 instead of "latest":

`.\Build-FFmpeg.ps1 -VsWhereCriteria '-version [15.0,16.0)'`


Build using Visual Studio 2019 instead of "latest":

`.\Build-FFmpeg.ps1 -VsWhereCriteria '-version [16.0,17.0)'`


Build only x86 and x64:

`.\Build-FFmpeg.ps1 -Platforms x86, x64`


You can of course combine parameters. There are more parameters, you can see them at the beginning of the build script.


Note: You need Visual Studio 2017 15.9.0 or higher to build the ARM64 version of ffmpeg!

## Building the FFmpegInterop library

Run Build-FFmpegInteropX.ps1 to build the FFmpegInteropX library.

Run the build script from PowerShell:

`.\Build-FFmpegInteropX.ps1`

Run the build script from CMD:

`PowerShell -NoProfile -ExecutionPolicy Bypass -Command ".\Build-FFmpegInteropX.ps1"`

This script has similar parameters as the `Build-FFmpeg.ps1` script. Check parameters above.

## Integrating the FFmpegInteropX library into your app solution (instead of using NuGet package)

If you want to integrate the FFmpegInteropX library into your app, you can just add the project file (`FFmpegInterop\FFmpegInterop.vcxproj`) to your app solution as an existing project and add a reference from your main app project to FFmpegInterop. The FFmpegInterop project does not have to be in your app's solution folder. This allows you to debug into the library.


## Integrating custom FFmpeg builds into your app solution using NuGet

When you build FFmpeg UWP and specify a NuGet package version, a package will be created automatically. When you open the FFmpegInteropX solution and click "Manage NuGet packages for Solution", select "LocalPackages" as package source in the top right area. Now you can easily select your latest build and install it into the sample projects. If you want to try the build in your own app, you need to create a NuGet.config file in your app's solution folder, similar to the one in our repo, and have it point to the FFmpegInteropX Output\NuGet folder.

## Integrating custom FFmpeg builds into your app solution without NuGet

Instead of using the FFmpegInteropX.FFmpegUWP NuGet package, you can also manually reference your custom built ffmpeg dll files for the platform you are building. Best is to manually edit your app's project file. This allows you to refer the dlls built for the current platform using $BuildPlatform parameter.

For a C# project, you can do it like this:

```
  <ItemGroup>
    <Content Include="$(SolutionDir)..\FFmpegInteropX\FFmpegUWP\$(PlatformTarget)\bin\*.dll" />
  </ItemGroup>
```

This assumes that the FFmpegInteropX repository is located next to your solution folder and the FFmpegUWP build output folder contains the dlls. Paths might be different on your folder setup. For CPP projects this can be done similarly, check the samples for reference. If your program crashes with `The specified module could not be found. (Exception from HRESULT: 0x8007007E)` error, the path is probably wrong. 
