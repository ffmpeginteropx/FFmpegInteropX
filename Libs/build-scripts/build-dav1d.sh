#!/bin/bash

DIR=$( pwd )

variant=$1
platform=$2

intDir="$DIR/Intermediate/FFmpeg$variant/$platform/int/dav1d"
outDir="$DIR/Intermediate/FFmpeg$variant/$platform"

# make sure path to link.exe (same dir as cl.exe) is at start of path variable
# this avoids conflicts with GNU link.exe
clpath=$(dirname "$(which cl.exe)")
PATH=$clpath:$PATH

cd Libs/dav1d

rm -rf $intDir

meson $intDir --cross-file ../build-scripts/dav1d-cross/$platform.txt || exit

cd $intDir
meson configure -Dbuildtype=release || exit
meson configure -Ddefault_library=static || exit
meson configure -Denable_tests=false || exit
meson configure -Denable_asm=true || exit

ninja || exit

mkdir -p $outDir/lib
mkdir -p $outDir/include
mkdir -p $outDir/include/dav1d
mkdir -p $outDir/licenses

cp $intDir/src/libdav1d.a $outDir/lib/libdav1d.lib || exit
cp $DIR/Libs/dav1d/include/dav1d/*.h $outDir/include/dav1d || exit
cp -r $intDir/include/dav1d $outDir/include || exit
cp $DIR/Libs/dav1d/COPYING $outDir/licenses/dav1d.txt
