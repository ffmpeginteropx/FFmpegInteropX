@setlocal
@pushd %~dp0
@echo off

:: Checking that we are running from a clean non-dev cmd

if defined VSINSTALLDIR (
    echo:
    echo This script must be run from a clean cmd environment. 
	echo Do NOT run from VisualStudio Command Prompt such as "ARM Cross Tools Command Prompt".
	echo:
	echo Found variable: VSINSTALLDIR
    goto Cleanup
)

if defined INCLUDE (
    echo:
    echo This script must be run from a clean cmd environment. 
	echo Do NOT run from VisualStudio Command Prompt such as "ARM Cross Tools Command Prompt".
	echo:
	echo Found variable: INCLUDE
    goto Cleanup
)

if defined LIB (
    echo:
    echo This script must be run from a clean cmd environment. 
	echo Do NOT run from VisualStudio Command Prompt such as "ARM Cross Tools Command Prompt".
	echo:
	echo Found variable: LIB
    goto Cleanup
)

if defined LIBPATH (
    echo:
    echo This script must be run from a clean cmd environment. 
	echo Do NOT run from VisualStudio Command Prompt such as "ARM Cross Tools Command Prompt".
	echo:
	echo Found variable: LIBPATH
    goto Cleanup
)

echo:
echo Searching VS Installation folder...

set VSInstallerFolder="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
if %PROCESSOR_ARCHITECTURE%==x86 set VSInstallerFolder="%ProgramFiles%\Microsoft Visual Studio\Installer"

pushd %VSInstallerFolder%
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set VSLATESTDIR=%%i
)
popd

echo VS Installation folder: %VSLATESTDIR%

if not exist "%VSLATESTDIR%\VC\Auxiliary\Build\vcvarsall.bat" (
    echo:
    echo VSInstallDir not found or not installed correctly.
    goto Cleanup
)

echo:
echo Checking CPU architecture...

if %PROCESSOR_ARCHITECTURE%==x86 (
    set Comp_x86=x86 uwp 10.0.15063.0
    set Comp_x64=x86_amd64 uwp 10.0.15063.0
    set Comp_ARM=x86_arm uwp 10.0.15063.0
    set Comp_ARM64=x86_arm64 uwp 10.0.15063.0
) else (
    set Comp_x86=amd64_x86 uwp 10.0.15063.0
    set Comp_x64=amd64 uwp 10.0.15063.0
    set Comp_ARM=amd64_arm uwp 10.0.15063.0
    set Comp_ARM64=amd64_arm64 uwp 10.0.15063.0
)

:: Export full current PATH from environment into MSYS2
set MSYS2_PATH_TYPE=inherit

:: Verifying ffmpeg directory
echo Verifying ffmpeg directory...
pushd %~dp0\ffmpeg
if not exist configure (
    echo:
    echo configure is not found in ffmpeg folder. Ensure this folder is populated with ffmpeg snapshot
    goto Cleanup
)
popd

:: Check for required tools

echo:
echo Checking for MSYS2...

if not defined MSYS2_BIN (
    if exist C:\msys64\usr\bin\bash.exe set MSYS2_BIN="C:\msys64\usr\bin\bash.exe"
)

if not defined MSYS2_BIN (
    if exist C:\msys\usr\bin\bash.exe set MSYS2_BIN="C:\msys\usr\bin\bash.exe"
)

if defined MSYS2_BIN (
    if exist %MSYS2_BIN% goto Win10
)

echo:
echo MSYS2 is needed. Set it up properly and provide the executable path in MSYS2_BIN environment variable. E.g.
echo:
echo     set MSYS2_BIN="C:\msys64\usr\bin\bash.exe"
echo:
echo See https://trac.ffmpeg.org/wiki/CompilationGuide/WinRT#PrerequisitesandFirstTimeSetupInstructions
goto Cleanup

:: Build and deploy Windows 10 library
:Win10

set OnePlatform=1

if "%1" == "" (
    echo:
	echo Building FFmpeg for all platforms
	set OnePlatform=0
	goto Win10x86
)
if %1 == x86 goto Win10x86
if %1 == x64 goto Win10x64
if %1 == ARM goto Win10ARM
if %1 == ARM64 goto Win10ARM64

echo Invalid argument: %1
goto Cleanup


:Win10x86
echo:
echo Building FFmpeg for Windows 10 apps x86...
echo:

setlocal
call "%VSLATESTDIR%\VC\Auxiliary\Build\vcvarsall.bat" %Comp_x86%

set ARCH=x86

:: Build libs
set libs=Libs\Build\%ARCH%
md %libs%\lib
md %libs%\licenses
md %libs%\include
rd /S /Q %libs%\build
md %libs%\build

msbuild Libs\zlib\SMP\libzlib.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libzlib\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libzlib\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.lib %libs%\lib\zlib.lib
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.pdb %libs%\lib\zlib.pdb

msbuild Libs\bzip2\SMP\libbz2.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libbz2\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libbz2\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.lib  %libs%\lib\bz2.lib
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.pdb  %libs%\lib\bz2.pdb

msbuild Libs\libiconv\SMP\libiconv.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libiconv\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libiconv\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.lib  %libs%\lib\iconv.lib
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.pdb  %libs%\lib\iconv.pdb

set LIB=%LIB%;%~dp0%libs%\lib
set INCLUDE=%INCLUDE%;%~dp0%libs%\include

:: Build ffmpeg
%MSYS2_BIN% --login -x "%~dp0FFmpegConfig.sh" Win10 %ARCH% || goto error
for /r ffmpeg\Output\Windows10\%ARCH% %%f in (*.pdb) do copy /Y "%%f" ffmpeg\Build\Windows10\%ARCH%\bin\
endlocal

if %OnePlatform% == 1 goto Cleanup



:Win10x64
echo:
echo Building FFmpeg for Windows 10 apps x64...
echo:

setlocal
call "%VSLATESTDIR%\VC\Auxiliary\Build\vcvarsall.bat" %Comp_x64%

set ARCH=x64

:: Build libs
set libs=Libs\Build\%ARCH%
md %libs%\lib
md %libs%\licenses
md %libs%\include
rd /S /Q %libs%\build
md %libs%\build

msbuild Libs\zlib\SMP\libzlib.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libzlib\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libzlib\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.lib %libs%\lib\zlib.lib
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.pdb %libs%\lib\zlib.pdb

msbuild Libs\bzip2\SMP\libbz2.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libbz2\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libbz2\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.lib  %libs%\lib\bz2.lib
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.pdb  %libs%\lib\bz2.pdb

msbuild Libs\libiconv\SMP\libiconv.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libiconv\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libiconv\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.lib  %libs%\lib\iconv.lib
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.pdb  %libs%\lib\iconv.pdb

set LIB=%LIB%;%~dp0%libs%\lib
set INCLUDE=%INCLUDE%;%~dp0%libs%\include

:: Build ffmpeg
%MSYS2_BIN% --login -x "%~dp0FFmpegConfig.sh" Win10 %ARCH% || goto error
for /r ffmpeg\Output\Windows10\%ARCH% %%f in (*.pdb) do copy /Y "%%f" ffmpeg\Build\Windows10\%ARCH%\bin\
endlocal

if %OnePlatform% == 1 goto Cleanup



:Win10ARM
echo:
echo Building FFmpeg for Windows 10 apps ARM...
echo:

setlocal
call "%VSLATESTDIR%\VC\Auxiliary\Build\vcvarsall.bat" %Comp_ARM%

set ARCH=ARM

:: Build libs
set libs=Libs\Build\%ARCH%
md %libs%\lib
md %libs%\licenses
md %libs%\include
rd /S /Q %libs%\build
md %libs%\build

msbuild Libs\zlib\SMP\libzlib.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libzlib\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libzlib\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.lib %libs%\lib\zlib.lib
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.pdb %libs%\lib\zlib.pdb

msbuild Libs\bzip2\SMP\libbz2.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libbz2\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libbz2\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.lib  %libs%\lib\bz2.lib
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.pdb  %libs%\lib\bz2.pdb

msbuild Libs\libiconv\SMP\libiconv.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libiconv\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libiconv\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.lib  %libs%\lib\iconv.lib
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.pdb  %libs%\lib\iconv.pdb

set LIB=%LIB%;%~dp0%libs%\lib
set INCLUDE=%INCLUDE%;%~dp0%libs%\include

:: Build ffmpeg
%MSYS2_BIN% --login -x "%~dp0FFmpegConfig.sh" Win10 %ARCH% || goto error
for /r ffmpeg\Output\Windows10\%ARCH% %%f in (*.pdb) do copy /Y "%%f" ffmpeg\Build\Windows10\%ARCH%\bin\
endlocal

if %OnePlatform% == 1 goto Cleanup



:Win10ARM64
echo:
echo Building FFmpeg for Windows 10 apps ARM64...
echo:

setlocal
call "%VSLATESTDIR%\VC\Auxiliary\Build\vcvarsall.bat" %Comp_ARM64%

set ARCH=ARM64

:: Build libs
set libs=Libs\Build\%ARCH%
md %libs%\lib
md %libs%\licenses
md %libs%\include
rd /S /Q %libs%\build
md %libs%\build

msbuild Libs\zlib\SMP\libzlib.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libzlib\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libzlib\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.lib %libs%\lib\zlib.lib
copy /Y %libs%\build\libzlib\lib\%ARCH%\libzlib_winrt.pdb %libs%\lib\zlib.pdb

msbuild Libs\bzip2\SMP\libbz2.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libbz2\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libbz2\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.lib  %libs%\lib\bz2.lib
copy /Y %libs%\build\libbz2\lib\%ARCH%\libbz2_winrt.pdb  %libs%\lib\bz2.pdb

msbuild Libs\libiconv\SMP\libiconv.vcxproj -p:OutDir="%~dp0%libs%\build\\";Configuration=ReleaseWinRT;Platform=%ARCH% || goto error
for /r %libs%\build\libiconv\include %%f in (*.h) do copy /Y "%%f" %libs%\include\
for /r %libs%\build\libiconv\licenses %%f in (*.*) do copy /Y "%%f" %libs%\licenses\
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.lib  %libs%\lib\iconv.lib
copy /Y %libs%\build\libiconv\lib\%ARCH%\libiconv_winrt.pdb  %libs%\lib\iconv.pdb

set LIB=%LIB%;%~dp0%libs%\lib
set INCLUDE=%INCLUDE%;%~dp0%libs%\include

:: Build ffmpeg
%MSYS2_BIN% --login -x "%~dp0FFmpegConfig.sh" Win10 %ARCH% || goto error
for /r ffmpeg\Output\Windows10\%ARCH% %%f in (*.pdb) do copy /Y "%%f" ffmpeg\Build\Windows10\%ARCH%\bin\
endlocal

if %OnePlatform% == 1 goto Cleanup



if %errorlevel% == 0 goto Cleanup


:error
echo:
echo Build failed with error %errorlevel%.
popd
endlocal
exit /b


:Cleanup
@popd
@endlocal