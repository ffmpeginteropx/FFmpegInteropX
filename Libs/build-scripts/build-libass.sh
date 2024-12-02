#!/bin/bash

DIR=$( pwd )

variant=$1
platform=$2

intDir="$DIR/Intermediate/FFmpeg$variant/$platform/int/libass"
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

cd Libs/libass

rm -rf $intDir

echo cross

PATH=$PATH /mingw64/bin/meson $intDir --cross-file ../build-scripts/libass-cross/$platform.txt || exit
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
mkdir -p $outDir/include/libass
mkdir -p $outDir/licenses

echo copy

cp $intDir/src/libass.a $outDir/lib/libass.lib || exit
cp $DIR/Libs/libass/include/libass/*.h $outDir/include/libass || exit
cp -r $intDir/include/libass $outDir/include || exit
cp $DIR/Libs/libass/COPYING $outDir/licenses/libass.txt
