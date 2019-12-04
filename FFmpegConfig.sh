#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# declare -A arch
# arch['x86']='x86'
# arch['x64']='x86_64'
# arch['ARM']='arm'
# arch['ARM64']='arm64'

# variant=$1
# platform=$2

# #Main
# echo "Make $1 $2"
# pushd $DIR/ffmpeg
# rm -rf $DIR/Target/$2/Release/ffmpeg-$1
# rm -rf $DIR/Build/$2/Release/ffmpeg-$1
# mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
# cd $DIR/Build/$2/Release/ffmpeg-$1
# $DIR/ffmpeg/configure \
#     --toolchain=msvc \
#     --disable-programs \
#     --disable-d3d11va \
#     --disable-dxva2 \
#     #--disable-encoders \ 4 => Win10
#     #--disable-devices \ 4 => Win10
#     #--disable-hwaccels \ 4 => Win10
#     --disable-doc \
#     --arch="${arch[$2]}" \
#     --enable-shared \
#     --enable-cross-compile \
#     --enable-debug \
#     #--enable-zlib \ 4 => Win10
#     #--enable-bzlib \ 4 => Win10
#     #--enable-iconv \ 4 => Win10
#     --target-os=win32 \
#     #--extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00" \ 2
#     #--extra-ldflags="-APPCONTAINER WindowsApp.lib" \ 4 =>Win10  x64 x86
#     --prefix=$DIR/Target/$2/Release/ffmpeg-$1
# #make -j8 4
# make install
# popd

if [ "$1" == "Win10" ]; then
    echo "Make $1"

    if [ "$2" == "x86" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Target/$2/Release/ffmpeg-$1
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-encoders \
        --disable-devices \
        --disable-hwaccels \
        --disable-doc \
        --arch="${arch[$2]}" \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --enable-zlib \
        --enable-bzlib \
        --enable-iconv \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00" \
        --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make -j8
        make install
        popd

    elif [ "$2" == "x64" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Target/$2/Release/ffmpeg-$1
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-encoders \
        --disable-devices \
        --disable-hwaccels \
        --disable-doc \
        --arch="${arch[$2]}" \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --enable-zlib \
        --enable-bzlib \
        --enable-iconv \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00" \
        --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make -j8
        make install
        popd

    elif [ "$2" == "ARM" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Target/$2/Release/ffmpeg-$1
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-encoders \
        --disable-devices \
        --disable-hwaccels \
        --disable-doc \
        --arch="${arch[$2]}" \
        --as=armasm \
        --cpu=armv7 \
        --enable-thumb \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --enable-zlib \
        --enable-bzlib \
        --enable-iconv \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -D__ARM_PCS_VFP" \
        --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make -j8
        make install
        popd

    elif [ "$2" == "ARM64" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Target/$2/Release/ffmpeg-$1
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-encoders \
        --disable-devices \
        --disable-hwaccels \
        --disable-doc \
        --arch="${arch[$2]}" \
        --cpu=armv7 \
        --enable-thumb \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --enable-zlib \
        --enable-bzlib \
        --enable-iconv \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -D__ARM_PCS_VFP" \
        --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make -j8
        make install
        popd

    fi

elif [ "$1" == "Win8.1" ]; then
    echo "Make $1"

    if [ "$2" == "x86" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch="${arch[$2]}" \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603" \
        --extra-ldflags="-APPCONTAINER" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make install
        popd

    elif [ "$2" == "x64" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch="${arch[$2]}" \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603" \
        --extra-ldflags="-APPCONTAINER" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make install
        popd

    elif [ "$2" == "ARM" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch="${arch[$2]}" \
        --as=armasm \
        --cpu=armv7 \
        --enable-thumb \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603 -D__ARM_PCS_VFP" \
        --extra-ldflags="-APPCONTAINER -MACHINE:$2" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make install
        popd

    fi

elif [ "$1" == "Phone8.1" ]; then
    echo "Make $1"

    if [ "$2" == "ARM" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch="${arch[$2]}" \
        --as=armasm \
        --cpu=armv7 \
        --enable-thumb \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PHONE_APP -D_WIN32_WINNT=0x0603 -D__ARM_PCS_VFP" \
        --extra-ldflags="-APPCONTAINER -MACHINE:$2 -subsystem:console -opt:ref WindowsPhoneCore.lib RuntimeObject.lib PhoneAppModelHost.lib -NODEFAULTLIB:kernel32.lib -NODEFAULTLIB:ole32.lib" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make install
        popd

    elif [ "$2" == "x86" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch="${arch[$2]}" \
        --enable-shared \
        --enable-cross-compile \
        --target-os=win32 \
        --enable-debug \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PHONE_APP -D_WIN32_WINNT=0x0603" \
        --extra-ldflags="-APPCONTAINER -subsystem:console -opt:ref WindowsPhoneCore.lib RuntimeObject.lib PhoneAppModelHost.lib -NODEFAULTLIB:kernel32.lib -NODEFAULTLIB:ole32.lib" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make install
        popd

    fi

elif [ "$1" == "Win7" ]; then
    echo "Make $1"

    if [ "$2" == "x86" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch="${arch[$2]}" \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -D_WINDLL" \
        --extra-ldflags="-APPCONTAINER:NO -MACHINE:$2" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make install
        popd

    elif [ "$2" == "x64" ]; then
        echo "Make $1 $2"
        pushd $DIR/ffmpeg
        rm -rf $DIR/Build/$2/Release/ffmpeg-$1
        mkdir -p $DIR/Build/$2/Release/ffmpeg-$1
        cd $DIR/Build/$2/Release/ffmpeg-$1
        $DIR/ffmpeg/configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch="${arch[$2]}" \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -D_WINDLL" \
        --extra-ldflags="-APPCONTAINER:NO -MACHINE:$2" \
        --prefix=$DIR/Target/$2/Release/ffmpeg-$1
        make install
        popd

    fi
fi

exit 0
