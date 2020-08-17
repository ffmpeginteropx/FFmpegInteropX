md Output\NuGet

nuget restore FFmpegInterop.sln

msbuild FFmpegInterop.sln /m /t:build /p:Configuration=Debug;Platform=x86;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild FFmpegInterop\FFmpegInterop.vcxproj /t:build /p:Configuration=Release;Platform=x86;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild FFmpegInterop\FFmpegInterop.vcxproj /t:build /p:Configuration=Release;Platform=x64;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild FFmpegInterop\FFmpegInterop.vcxproj /t:build /p:Configuration=Release;Platform=ARM;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild FFmpegInterop\FFmpegInterop.vcxproj /t:build /p:Configuration=Release;Platform=ARM64;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

nuget pack FFmpegInteropX.nuspec -Properties id=FFmpegInteropX -Version %1 -Symbols -SymbolPackageFormat snupkg