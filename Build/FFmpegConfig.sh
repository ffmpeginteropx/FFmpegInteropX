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

if [ "$encoders" == "enable" ] && [ "$gpl" == "enable" ]; then
    gplEncoders="enable"
else
    gplEncoders="disable"
fi

intDir="$DIR/Intermediate/$folderName/$platform/int/ffmpeg"
outDir="$DIR/Output/$folderName/$platform"

#Main
echo "Make $variant $platform $sharedOrStatic $gpl-glp $encoders-encoders $devices-encoders $programs-programs" $8
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
    --enable-libdav1d  \
    --enable-openssl \
    --enable-hwaccels \
    --enable-d3d11va \
    --disable-dxva2 \
    --${encoders}-encoders \
    --${gplEncoders}-libx264 \
    --${gplEncoders}-libx265 \
    --${encoders}-libvpx \
    --${devices}-devices \
    --${gpl}-gpl \
    --${gpl}-version3 \
    --${programs}-programs \
    --target-os=win32 \
    --pkg-config=$DIR/Intermediate/pkg-config.exe \
    --prefix=$outDir \
"

cflags="-MD -DLZMA_API_STATIC"
ldflags=""

if [ "$variant" == "UWP" ]; then

    ldflags="$ldflags -APPCONTAINER WindowsApp.lib"

    cflags="\
        $cflags \
        -DWINAPI_FAMILY=WINAPI_FAMILY_APP \
        -D_WIN32_WINNT=0x0A00
    "
fi

if [ "$variant" == "Desktop" ]; then

    ldflags="$ldflags -APPCONTAINER:NO -MACHINE:$platform Ws2_32.lib Advapi32.lib User32.lib"

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
    --extra-ldflags='$ldflags'
"

if [ "$9" != "-SkipConfigure" ]; then
    eval $DIR/Libs/ffmpeg/configure $configureArgs || exit 1
fi

make -j$(nproc) || exit
make install || exit

exit 0
