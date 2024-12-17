#!/bin/bash

DIR=$( pwd )

variant=$1
platform=$2

intDir="$DIR/Intermediate/FFmpeg$variant/$platform/int/freetype2"
outDir="$DIR/Intermediate/FFmpeg$variant/$platform"

# make sure path to link.exe (same dir as cl.exe) is at start of path variable
# this avoids conflicts with GNU link.exe
clpath=$(which cl)
clpath="${clpath::-3}"
PATH=$DIR/Intermediate:$clpath:$PATH

echo $PATH
echo $(which pkg-config)
echo $(which pkg-config.exe)

export PKG_CONFIG_PATH=$DIR/Intermediate
#alias pkg-config=$DIR/Intermediate/pkg-config.exe

export PATH=$PATH

cd Libs/freetype2

rm -rf $intDir

echo cross

PATH=$PATH /mingw64/bin/meson $intDir --cross-file ../build-scripts/freetype2-cross/$platform.txt || exit
echo configure

cd $intDir

/mingw64/bin/meson configure -Dbuildtype=release || exit
/mingw64/bin/meson configure -Ddefault_library=static || exit
/mingw64/bin/meson configure -Denable_tests=false || exit
/mingw64/bin/meson configure -Denable_asm=true || exit

echo ninja
/mingw64/bin/ninja || exit

mkdir -p $outDir/lib
mkdir -p $outDir/include
mkdir -p $outDir/include/freetype2
mkdir -p $outDir/licenses

echo copy

cp $intDir/src/freetype2.a $outDir/lib/freetype2.lib || exit
cp $DIR/Libs/freetype2/include/freetype2/*.h $outDir/include/freetype2 || exit
cp -r $intDir/include/freetype2 $outDir/include || exit
cp $DIR/Libs/freetype2/COPYING $outDir/licenses/freetype2.txt
