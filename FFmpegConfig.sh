#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ "$1" == "Win10" ]; then
    echo "Make Win10"

    if [ "$2" == "x86" ]; then
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
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00" \
        --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
        --prefix=../../../Build/Windows10/x86
        make -j8
        make install
        popd

    elif [ "$2" == "x64" ]; then
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
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00" \
        --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
        --prefix=../../../Build/Windows10/x64
        make -j8
        make install
        popd

    elif [ "$2" == "ARM" ]; then
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
        --enable-thumb \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_WIN32_WINNT=0x0A00 -D__ARM_PCS_VFP" \
        --extra-ldflags="-APPCONTAINER WindowsApp.lib" \
        --prefix=../../../Build/Windows10/ARM
        make -j8
        make install
        popd

    fi

elif [ "$1" == "Win8.1" ]; then
    echo "Make Win8.1"

    if [ "$2" == "x86" ]; then
        echo "Make Win8.1 x86"
        pushd $DIR/ffmpeg
        rm -rf Output/Windows8.1/x86
        mkdir -p Output/Windows8.1/x86
        cd Output/Windows8.1/x86
        ../../../configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch=x86 \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603" \
        --extra-ldflags="-APPCONTAINER" \
        --prefix=../../../Build/Windows8.1/x86
        make install
        popd

    elif [ "$2" == "x64" ]; then
        echo "Make Win8.1 x64"
        pushd $DIR/ffmpeg
        rm -rf Output/Windows8.1/x64
        mkdir -p Output/Windows8.1/x64
        cd Output/Windows8.1/x64
        ../../../configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch=x86_64 \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603" \
        --extra-ldflags="-APPCONTAINER" \
        --prefix=../../../Build/Windows8.1/x64
        make install
        popd

    elif [ "$2" == "ARM" ]; then
        echo "Make Win8.1 ARM"
        pushd $DIR/ffmpeg
        rm -rf Output/Windows8.1/ARM
        mkdir -p Output/Windows8.1/ARM
        cd Output/Windows8.1/ARM
        ../../../configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch=arm \
        --as=armasm \
        --cpu=armv7 \
        --enable-thumb \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PC_APP -D_WIN32_WINNT=0x0603 -D__ARM_PCS_VFP" \
        --extra-ldflags="-APPCONTAINER -MACHINE:ARM" \
        --prefix=../../../Build/Windows8.1/ARM
        make install
        popd

    fi

elif [ "$1" == "Phone8.1" ]; then
    echo "Make Phone8.1"

    if [ "$2" == "ARM" ]; then
        echo "Make Phone8.1 ARM"
        pushd $DIR/ffmpeg
        rm -rf Output/WindowsPhone8.1/ARM
        mkdir -p Output/WindowsPhone8.1/ARM
        cd Output/WindowsPhone8.1/ARM
        ../../../configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch=arm \
        --as=armasm \
        --cpu=armv7 \
        --enable-thumb \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PHONE_APP -D_WIN32_WINNT=0x0603 -D__ARM_PCS_VFP" \
        --extra-ldflags="-APPCONTAINER -MACHINE:ARM -subsystem:console -opt:ref WindowsPhoneCore.lib RuntimeObject.lib PhoneAppModelHost.lib -NODEFAULTLIB:kernel32.lib -NODEFAULTLIB:ole32.lib" \
        --prefix=../../../Build/WindowsPhone8.1/ARM
        make install
        popd

    elif [ "$2" == "x86" ]; then
        echo "Make Phone8.1 x86"
        pushd $DIR/ffmpeg
        rm -rf Output/WindowsPhone8.1/x86
        mkdir -p Output/WindowsPhone8.1/x86
        cd Output/WindowsPhone8.1/x86
        ../../../configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch=x86 \
        --enable-shared \
        --enable-cross-compile \
        --target-os=win32 \
        --enable-debug \
        --extra-cflags="-MD -DWINAPI_FAMILY=WINAPI_FAMILY_PHONE_APP -D_WIN32_WINNT=0x0603" \
        --extra-ldflags="-APPCONTAINER -subsystem:console -opt:ref WindowsPhoneCore.lib RuntimeObject.lib PhoneAppModelHost.lib -NODEFAULTLIB:kernel32.lib -NODEFAULTLIB:ole32.lib" \
        --prefix=../../../Build/WindowsPhone8.1/x86
        make install
        popd

    fi

elif [ "$1" == "Win7" ]; then
    echo "Make Win7"

    if [ "$2" == "x86" ]; then
        echo "Make Win7 x86"
        pushd $DIR/ffmpeg
        rm -rf Output/Windows7/x86
        mkdir -p Output/Windows7/x86
        cd Output/Windows7/x86
        ../../../configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch=x86 \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -D_WINDLL" \
        --extra-ldflags="-APPCONTAINER:NO -MACHINE:x86" \
        --prefix=../../../Build/Windows7/x86
        make install
        popd

    elif [ "$2" == "x64" ]; then
        echo "Make Win7 x64"
        pushd $DIR/ffmpeg
        rm -rf Output/Windows7/x64
        mkdir -p Output/Windows7/x64
        cd Output/Windows7/x64
        ../../../configure \
        --toolchain=msvc \
        --disable-programs \
        --disable-d3d11va \
        --disable-dxva2 \
        --disable-doc \
        --arch=amd64 \
        --enable-shared \
        --enable-cross-compile \
        --enable-debug \
        --target-os=win32 \
        --extra-cflags="-MD -D_WINDLL" \
        --extra-ldflags="-APPCONTAINER:NO -MACHINE:x64" \
        --prefix=../../../Build/Windows7/x64
        make install
        popd

    fi
fi

exit 0