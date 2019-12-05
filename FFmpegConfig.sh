#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

declare -A arch
arch['x86']='x86'
arch['x64']='x86_64'
arch['ARM']='arm'
arch['ARM64']='arm64'

variant=$1
platform=$2

#Main
echo "Make $variant $platform"
pushd $DIR/ffmpeg
rm -rf $DIR/Target/$platform/Release/ffmpeg-$variant
rm -rf $DIR/Build/$platform/Release/ffmpeg-$variant
mkdir -p $DIR/Build/$platform/Release/ffmpeg-$variant
cd $DIR/Build/$platform/Release/ffmpeg-$variant
configureArgs=" \
    --toolchain=msvc \
    --disable-programs \
    --disable-d3d11va \
    --disable-dxva2 \
    --disable-doc \
    --arch=\"${arch[$platform]}\" \
    --enable-shared \
    --enable-cross-compile \
    --enable-debug \
    --target-os=win32 \
    --prefix=$DIR/Target/$platform/Release/ffmpeg-$variant
"
makeArgs=''

if [ "$variant" == "Win10" ]; then
    configureArgs="$configureArgs \
        --disable-encoders \
        --disable-devices \
        --disable-hwaccels \
        --enable-zlib \
        --enable-bzlib \
        --enable-iconv \
        --extra-ldflags=\"-APPCONTAINER WindowsApp.lib\"
"

    makeArgs="$makeArgs -j8"

    if [ "$platform" = "x64" ] || [ "$platform" = "x86" ]; then
        configureArgs="$configureArgs \
            --extra-cflags=\"-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00\"
"
    fi

    if [ "$platform" = "ARM" ] || [ "$platform" = "ARM64" ]; then
        configureArgs="$configureArgs
            --extra-cflags=\"-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -D__ARM_PCS_VFP\"
            "
    fi
fi

if [ "$variant" == "Win8.1" ]; then
    if [ "$platform" = "x64" ] || [ "$platform" = "x86" ]; then
        configureArgs="$configureArgs
            --extra-cflags=\"-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603\"
            --extra-ldflags=\"-APPCONTAINER\"
            "
    fi

    if [ "$platform" == "ARM" ]; then
        configureArgs="$configureArgs
            --extra-cflags=\"-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603 -D__ARM_PCS_VFP\"
            --extra-ldflags=\"-APPCONTAINER -MACHINE:$platform\"
            "
    fi
fi

if [ "$variant" == "Phone8.1" ]; then
    if [ "$platform" == "ARM" ]; then
        configureArgs="$configureArgs
            --extra-cflags=\"-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PHONE_APP -D_WIN32_WINNT=0x0603 -D__ARM_PCS_VFP\"
            --extra-ldflags=\"-APPCONTAINER -MACHINE:$platform -subsystem:console -opt:ref WindowsPhoneCore.lib RuntimeObject.lib PhoneAppModelHost.lib -NODEFAULTLIB:kernel32.lib -NODEFAULTLIB:ole32.lib\"
            "

    elif [ "$platform" == "x86" ]; then
        configureArgs="$configureArgs
            --extra-cflags=\"-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PHONE_APP -D_WIN32_WINNT=0x0603\"
            --extra-ldflags=\"-APPCONTAINER -subsystem:console -opt:ref WindowsPhoneCore.lib RuntimeObject.lib PhoneAppModelHost.lib -NODEFAULTLIB:kernel32.lib -NODEFAULTLIB:ole32.lib\"
            "
    fi
fi

if [ "$variant" == "Win7" ]; then
    configureArgs="$configureArgs \
        --extra-cflags=\"-MD -D_WINDLL\" \
        --extra-ldflags=\"-APPCONTAINER:NO -MACHINE:$platform\"
    "
fi

if [ "$platform" == "ARM" ]; then
    configureArgs="$configureArgs \
        --as=armasm \
        --cpu=armv7 \
        --enable-thumb
    "
fi

if [ "$platform" = "ARM64" ]; then
    configureArgs="$configureArgs \
        --cpu=armv7 \
        --enable-thumb
    "
fi

eval "$DIR/ffmpeg/configure $configureArgs"

make $makeArgs
make install
popd

exit 0
