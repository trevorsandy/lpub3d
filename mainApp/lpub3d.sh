#!/bin/sh
# This script provides an alternative way of starting LPub3D.
# You can run this script instead of the executable to ensure
# depended upon libraries are found by the dynamic linker at
# application launch

appname=`basename $0 | sed s,\.sh$,,`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname/
fi
LD_LIBRARY_PATH=$dirname
export LD_LIBRARY_PATH
$dirname/$appname "$@"
