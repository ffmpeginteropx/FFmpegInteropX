md Output\NuGet

nuget restore FFmpegInteropX.sln

REM msbuild FFmpegInteropX.sln /m /t:build /p:Configuration=Debug;Platform=x64;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

msbuild FFmpegInteropX.sln /m /restore /t:build /p:Configuration=Debug_Desktop /p:Platform=x64 /p:AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

vstest.console /logger:Appveyor Output\FFmpegInteropX.UnitTests\x64\Debug\FFmpegInteropX.UnitTests.dll || exit

REM msbuild Source\FFmpegInteropX.DotNet.csproj /restore /t:build /p:Configuration=Release;Platform=AnyCPU;AppxBundle=Never;WINUI=1 /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

REM msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=x86;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

REM msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=x64;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

REM msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=ARM;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

REM msbuild Source\FFmpegInteropX.vcxproj /t:build /p:Configuration=Release;Platform=ARM64;AppxBundle=Never /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll" || exit

REM nuget pack Build\FFmpegInteropX.nuspec -Properties id=FFmpegInteropX -Version %1 -Symbols -SymbolPackageFormat snupkg
