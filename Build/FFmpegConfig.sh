#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )

export TERM=dumb

declare -A arch
arch['x86']='x86'
arch['x64']='x86_64'
arch['ARM']='arm'
arch['ARM64']='arm64'

variant=$1
platform=$2
sharedOrStatic=$3
gpl=$4
encoders=$5
devices=$6
programs=$7
folderName=$8

intDir="$DIR/Intermediate/$folderName/$platform/int/ffmpeg"
outDir="$DIR/Output/$folderName/$platform"

#Main
echo "Make $variant $platform $sharedOrStatic $gpl-glp $encoders-encoders $devices-encoders $programs-programs" $8
mkdir -p $intDir
mkdir -p $outDir
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
    --enable-libdav1d  \
    --enable-openssl \
    --enable-hwaccels \
    --enable-d3d11va \
    --disable-dxva2 \
    --${gpl}-gpl \
    --${gpl}-version3 \
    --target-os=win32 \
    --pkg-config=$DIR/Intermediate/pkg-config.exe \
    --prefix=$outDir \
"

if [ "$encoders" == "enable" ]; then
    if [ "$gpl" == "enable" ]; then
        configureArgs="$configureArgs --enable-libvpx --enable-libx264 --enable-libx265"
    else
        configureArgs="$configureArgs --enable-libvpx"
    fi
else
    configureArgs="$configureArgs --disable-encoders"
fi

if [ "$programs" == "disable" ]; then
    configureArgs="$configureArgs --disable-programs"
fi

if [ "$devices" == "disable" ]; then
    configureArgs="$configureArgs --disable-devices"
fi

cflags="-MD -DLZMA_API_STATIC"
ldflags=""

if [ "$variant" == "UWP" ]; then

    configureArgs="$configureArgs --disable-filter=gfxcapture"

    ldflags="$ldflags -APPCONTAINER WindowsApp.lib dxguid.lib"

    cflags="\
        $cflags \
        -DWINAPI_FAMILY=WINAPI_FAMILY_APP \
        -D_WIN32_WINNT=0x0A00
    "
fi

if [ "$variant" == "Desktop" ]; then

    ldflags="$ldflags -APPCONTAINER:NO -MACHINE:$platform Ws2_32.lib Advapi32.lib User32.lib dxguid.lib"

    if [ "$sharedOrStatic" = "shared" ]; then
        cflags="$cflags -D_WINDLL"
    fi
fi

if [ "$platform" == "ARM" ]; then

    cflags="$cflags -D__ARM_PCS_VFP" # only relevant for ARM 32 bit

    configureArgs="\
        $configureArgs \
        --as=armasm.exe \
        --cpu=armv7
    "
fi

if [ "$platform" = "ARM64" ]; then
    configureArgs="\
        $configureArgs \
        --as=armasm64.exe \
        --cpu=armv8
    "
fi

# disable some very common ffmpeg compiler warnings
cflags="$cflags -wd\"4267\" -wd\"4133\" -wd\"4334\" -wd\"4101\" "

configureArgs="\
    $configureArgs \
    --extra-cflags='$cflags' \
    --extra-cxxflags='$cflags' \
    --extra-ldflags='$ldflags'
"

# Perform configure (unless skipped)
if [ "$9" != "-SkipConfigure" ]; then
    eval $DIR/Libs/ffmpeg/configure $configureArgs || exit 1
fi

# Perform the build
make -j$(nproc) || exit

# Clean output directory before install
rm -r $outDir/*
mkdir $outDir/licenses

# Install files
make install || exit

exit 0
