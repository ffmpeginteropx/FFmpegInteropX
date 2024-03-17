set x265src=%~f1
set x265out=%~f2

REM assembly code for high bit depth only works on x64
if %3==x64 (
  set highAsm=ON
) else (
  set highAsm=OFF
)

if %3==ARM64 (
  set haveNeon=-DHAVE_NEON=1
  set lowAsm=OFF
  set toolchain=-DCMAKE_TOOLCHAIN_FILE=..\..\build-x265-crosscompile.cmake
) else (
  set haveNeon=
  set lowAsm=ON
  set toolchain=
)

cd /D %x265out%

@mkdir 12bit
@mkdir 10bit
@mkdir 8bit

@cd 12bit
cmake -G "Visual Studio 17 2022" -A %3 %x265src% %toolchain% -DCMAKE_GENERATOR_TOOLSET=%4 -DENABLE_ASSEMBLY=%highAsm% -DHIGH_BIT_DEPTH=ON -DEXPORT_C_API=OFF -DENABLE_SHARED=OFF -DENABLE_CLI=OFF -DMAIN12=ON
if exist x265.sln (
  MSBuild /property:Configuration="Release" /p:Platform="%3" x265.sln || exit /b
  copy/y Release\x265-static.lib ..\8bit\x265-static-main12.lib
)

@cd ..\10bit
cmake -G "Visual Studio 17 2022" -A %3 %x265src% %toolchain% -DCMAKE_GENERATOR_TOOLSET=%4 -DENABLE_ASSEMBLY=%highAsm% -DHIGH_BIT_DEPTH=ON -DEXPORT_C_API=OFF -DENABLE_SHARED=OFF -DENABLE_CLI=OFF
if exist x265.sln (
  MSBuild /property:Configuration="Release" /p:Platform="%3" x265.sln || exit /b
  copy/y Release\x265-static.lib ..\8bit\x265-static-main10.lib
)

@cd ..\8bit
if not exist x265-static-main10.lib (
  echo "10bit build failed"
  exit 1 /b
)
if not exist x265-static-main12.lib (
  echo "12bit build failed"
  exit 1 /b
)
cmake -G "Visual Studio 17 2022" -A %3 %x265src% %toolchain% -DCMAKE_GENERATOR_TOOLSET=%4 -DENABLE_ASSEMBLY=%lowAsm% -DCMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD=ON -DCMAKE_INSTALL_PREFIX:STRING=%x265out% -DEXTRA_LIB="x265-static-main10.lib;x265-static-main12.lib" -DLINKED_10BIT=ON -DLINKED_12BIT=ON -DEXPORT_C_API=ON -DENABLE_SHARED=ON -DENABLE_CLI=OFF
if exist x265.sln (
  MSBuild /property:Configuration="Release" /p:Platform="%3" x265.sln || exit /b
  :: combine static libraries (ignore warnings caused by winxp.cpp hacks)
  move Release\x265-static.lib x265-static-main.lib
  LIB.EXE /ignore:4006 /ignore:4221 /OUT:..\x265-static.lib x265-static-main.lib x265-static-main10.lib x265-static-main12.lib || exit /b
)
