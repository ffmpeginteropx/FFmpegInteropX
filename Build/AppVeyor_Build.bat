md Output\NuGet

nuget restore FFmpegInteropX.sln

msbuild FFmpegInteropX.sln /m /t:build /p:Configuration=Debug;Platform=x86;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild Source\FFmpegInteropX.DotNet.csproj /t:build /p:Configuration=Release;Platform=AnyCPU;AppxBundle=Never;WINUI=1 /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=x86;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=x64;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=ARM;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=ARM64;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

nuget pack Build\FFmpegInteropX.nuspec -Properties id=FFmpegInteropX -Version %1 -Symbols -SymbolPackageFormat snupkg