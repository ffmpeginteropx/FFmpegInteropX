#!/bin/bash

DIR=$( pwd )

variant=$1
platform=$2

intDir="$DIR/Intermediate/FFmpeg$variant/$platform/int/dav1d"
outDir="$DIR/Intermediate/FFmpeg$variant/$platform"

# make sure path to link.exe (same dir as cl.exe) is at start of path variable
# this avoids conflicts with GNU link.exe
clpath=$(which cl)
clpath="${clpath::-3}"
PATH=$clpath:$PATH

cd Libs/dav1d

rm -rf $intDir

/mingw64/bin/meson $intDir --cross-file ../build-scripts/dav1d-cross/$platform.txt || exit

cd $intDir
/mingw64/bin/meson configure -Dbuildtype=release || exit
/mingw64/bin/meson configure -Ddefault_library=static || exit
/mingw64/bin/meson configure -Denable_tests=false || exit
/mingw64/bin/meson configure -Denable_asm=true || exit

/mingw64/bin/ninja || exit

mkdir -p $outDir/lib
mkdir -p $outDir/include
mkdir -p $outDir/include/dav1d
mkdir -p $outDir/licenses

cp $intDir/src/libdav1d.a $outDir/lib/libdav1d.lib || exit
cp $DIR/Libs/dav1d/include/dav1d/*.h $outDir/include/dav1d || exit
cp -r $intDir/include/dav1d $outDir/include || exit
cp $DIR/Libs/dav1d/COPYING $outDir/licenses/dav1d.txt
