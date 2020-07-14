#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

declare -A arch
arch['x86']='x86'
arch['x64']='x86_64'
arch['ARM']='arm'
arch['ARM64']='arm64'

echo "Make Win10"

if [ "$1" == "x86" ]; then
    echo "Make Win10 x86"
    pushd $DIR/ffmpeg
    rm -rf Build/Windows10/x86
    rm -rf Output/Windows10/x86
    mkdir -p Output/Windows10/x86
    cd Output/Windows10/x86
    ../../../configure \
    --toolchain=msvc \
    --disable-programs \
    --disable-d3d11va \
    --disable-dxva2 \
    --disable-encoders \
    --disable-devices \
    --disable-hwaccels \
    --disable-doc \
    --arch=x86 \
    --enable-shared \
    --enable-cross-compile \
    --enable-debug \
    --enable-zlib \
    --enable-bzlib \
    --enable-lzma \
    --enable-libxml2 \
    --enable-iconv \
    --target-os=win32 \
    --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -DLZMA_API_STATIC" \
    --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
    --pkg-config="$DIR/Libs/Build/pkg-config.exe" \
    --prefix=../../../Build/Windows10/x86
    make -j8 || exit
    make install || exit
    popd

elif [ "$1" == "x64" ]; then
    echo "Make Win10 x64"
    pushd $DIR/ffmpeg
    rm -rf Build/Windows10/x64
    rm -rf Output/Windows10/x64
    mkdir -p Output/Windows10/x64
    cd Output/Windows10/x64
    ../../../configure \
    --toolchain=msvc \
    --disable-programs \
    --disable-d3d11va \
    --disable-dxva2 \
    --disable-encoders \
    --disable-devices \
    --disable-hwaccels \
    --disable-doc \
    --arch=x86_64 \
    --enable-shared \
    --enable-cross-compile \
    --enable-debug \
    --enable-zlib \
    --enable-bzlib \
    --enable-lzma \
    --enable-libxml2 \
    --enable-iconv \
    --target-os=win32 \
    --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -DLZMA_API_STATIC" \
    --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
    --pkg-config="$DIR/Libs/Build/pkg-config.exe" \
    --prefix=../../../Build/Windows10/x64
    make -j8 || exit
    make install || exit
    popd

elif [ "$1" == "ARM" ]; then
    echo "Make Win10 ARM"
    pushd $DIR/ffmpeg
    rm -rf Build/Windows10/ARM
    rm -rf Output/Windows10/ARM
    mkdir -p Output/Windows10/ARM
    cd Output/Windows10/ARM
    ../../../configure \
    --toolchain=msvc \
    --disable-programs \
    --disable-d3d11va \
    --disable-dxva2 \
    --disable-encoders \
    --disable-devices \
    --disable-hwaccels \
    --disable-doc \
    --arch=arm \
    --as=armasm \
    --cpu=armv7 \
    --enable-shared \
    --enable-cross-compile \
    --enable-debug \
    --enable-zlib \
    --enable-bzlib \
    --enable-lzma \
    --enable-libxml2 \
    --enable-iconv \
    --target-os=win32 \
    --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -DLZMA_API_STATIC -D__ARM_PCS_VFP" \
    --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
    --pkg-config="$DIR/Libs/Build/pkg-config.exe" \
    --prefix=../../../Build/Windows10/ARM
    make -j8 || exit
    make install || exit
    popd

elif [ "$1" == "ARM64" ]; then
    echo "Make Win10 ARM64"
    pushd $DIR/ffmpeg
    rm -rf Build/Windows10/ARM64
    rm -rf Output/Windows10/ARM64
    mkdir -p Output/Windows10/ARM64
    cd Output/Windows10/ARM64
    ../../../configure \
    --toolchain=msvc \
    --disable-programs \
    --disable-d3d11va \
    --disable-dxva2 \
    --disable-encoders \
    --disable-devices \
    --disable-hwaccels \
    --disable-doc \
    --arch=arm64 \
    --as=armasm64 \
    --cpu=armv8 \
    --enable-shared \
    --enable-cross-compile \
    --enable-debug \
    --enable-zlib \
    --enable-bzlib \
    --enable-lzma \
    --enable-libxml2 \
    --enable-iconv \
    --target-os=win32 \
    --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -DLZMA_API_STATIC -D__ARM_PCS_VFP" \
    --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
    --pkg-config="$DIR/Libs/Build/pkg-config.exe" \
    --prefix=../../../Build/Windows10/ARM64
    make -j8 || exit
    make install || exit
    popd

fi

exit 0