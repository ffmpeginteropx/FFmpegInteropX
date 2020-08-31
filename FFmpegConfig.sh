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

intDir="$DIR/Intermediate/FFmpeg$variant/$platform/int/ffmpeg"
outDir="$DIR/Output/FFmpeg$variant/$platform"

#Main
echo "Make $variant $platform $sharedOrStatic $intDir $outDir"
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
    --pkg-config=$DIR/Intermediate/pkg-config.exe \
    --prefix=$outDir \
"

cflags="-MD -DLZMA_API_STATIC"

if [ "$variant" == "UWP" ]; then
    configureArgs="\
        $configureArgs \
        --disable-encoders \
        --disable-devices \
        --disable-hwaccels \
        --disable-programs \
        --extra-ldflags='-APPCONTAINER WindowsApp.lib' \
        "

    cflags="\
        $cflags \
        -DWINAPI_FAMILY=WINAPI_FAMILY_APP \
        -D_WIN32_WINNT=0x0A00
    "

    if [ "$platform" = "x64" ] || [ "$platform" = "x86" ]; then
        configureArgs="\
            $configureArgs \
            --extra-cflags='$cflags'
"
    fi

    if [ "$platform" = "ARM" ] || [ "$platform" = "ARM64" ]; then
        configureArgs="\
            $configureArgs \
            --extra-cflags='$cflags -D__ARM_PCS_VFP'
            "
    fi
fi

if [ "$variant" == "Desktop" ]; then
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
            --extra-cflags='$cflags -D_WINDLL' \
        "
    else
        configureArgs="\
            $configureArgs \
            --extra-cflags='$cflags' \
        "
    fi
fi

if [ "$platform" == "ARM" ]; then
    configureArgs="\
        $configureArgs \
        --arch=arm \
        --as=armasm.exe \
        --cpu=armv7
    "
fi

if [ "$platform" = "ARM64" ]; then
    configureArgs="\
        $configureArgs \
        --arch=arm64 \
        --as=armasm64.exe \
        --cpu=armv8
    "
fi

eval $DIR/Libs/ffmpeg/configure $configureArgs || exit 1

make -j8 || exit
make install || exit

exit 0
