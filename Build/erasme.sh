#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )

declare -A arch
arch['x86']='x86'
arch['x64']='x86_64'
arch['ARM']='arm'
arch['ARM64']='arm64'

intDir="$DIR/Intermediate/FFmpeg\$variant/\$platform/int/ffmpeg"
outDir="$DIR/Output/FFmpeg\$variant/\$platform"

#Main
echo "Make \$variant \$platform \$sharedOrStatic $intDir $outDir"
mkdir -p $intDir
cd $intDir

if [ "$WSL_DISTRO_NAME" != "" ]; then
    echo "Using WSL: [$WSL_DISTRO_NAME]"
else
    echo "no WSL"
fi

cflags="-MD -DLZMA_API_STATIC"

echo "cflags:"
echo $cflags
echo 'DIR:'
echo $DIR
echo 'intDir:'
echo $intDir
echo 'outDir:'
echo $outDir
echo 'FOO'
echo "[$FOO]"

exit 0
