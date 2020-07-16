#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

declare -A arch
arch['x86']='x86'
arch['x64']='x86_64'
arch['ARM']='arm'
arch['ARM64']='arm64'

variant=$1
platform=$2
sharedOrStatic=$3

intDir="$DIR/Output/FFmpeg$variant/$platform/int/ffmpeg"
outDir="$DIR/FFmpeg$variant/$platform"

#Main
echo "Make $variant $platform $sharedOrStatic $intDir $outDir"
pushd $DIR/ffmpeg
mkdir -p $intDir
cd $intDir

configureArgs="\
    --toolchain=msvc \
    --disable-doc \
    --arch=${arch[$platform]} \
    --enable-${sharedOrStatic} \
    --enable-cross-compile \
    --enable-debug \
    --enable-zlib \
    --enable-bzlib \
    --enable-lzma \
    --enable-libxml2 \
    --enable-iconv \
    --target-os=win32 \
    --pkg-config=$DIR/Output/pkg-config.exe \
    --prefix=$outDir \
"

if [ "$variant" == "UWP" ]; then
    configureArgs="\
        $configureArgs \
        --disable-encoders \
        --disable-devices \
        --disable-hwaccels \
        --disable-programs \
        --extra-ldflags='-APPCONTAINER WindowsApp.lib' \
        "

    if [ "$platform" = "x64" ] || [ "$platform" = "x86" ]; then
        configureArgs="\
            $configureArgs \
            --extra-cflags='-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -DLZMA_API_STATIC'
"
    fi

    if [ "$platform" = "ARM" ] || [ "$platform" = "ARM64" ]; then
        configureArgs="\
            $configureArgs \
            --extra-cflags='-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -DLZMA_API_STATIC -D__ARM_PCS_VFP'
            "
    fi
fi

if [ "$variant" == "Win32" ]; then
    configureArgs="\
        $configureArgs \
        --enable-libx264 \
        --enable-libx265 \
        --enable-libvpx \
        --enable-gpl \
        --enable-static \
        --extra-ldflags='-APPCONTAINER:NO -MACHINE:$platform'
    "

    if [ "$sharedOrStatic" = "shared" ]; then
        configureArgs="\
            $configureArgs \
            --extra-cflags='-MD -D_WINDLL -DLZMA_API_STATIC' \
        "
    else
        configureArgs="\
            $configureArgs \
            --extra-cflags='-MD -DLZMA_API_STATIC' \
        "
    fi
fi

if [ "$platform" == "ARM" ]; then
    configureArgs="\
        $configureArgs \
        --arch=arm \
        --as=armasm \
        --cpu=armv7
    "
fi

if [ "$platform" = "ARM64" ]; then
    configureArgs="\
        $configureArgs \
        --arch=arm64 \
        --as=armasm64 \
        --cpu=armv8
    "
fi

eval $DIR/Libs/ffmpeg/configure $configureArgs || exit

make -j8 || exit
make install || exit
popd

exit 0
