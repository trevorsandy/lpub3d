#!/bin/bash
#
# Remove pre-compiled header behaviour from lcLib.

# This script is used exclusively on Open Build Service
# for platforms that are hardened which is to say
# GCC's PCH mechanism is fundamentally incompatible
# with the security hardening requirements for RedHat
# based systems using Position Independent Executables (PIE).
# For example CentOS 8, RHEL 8 and OpenEuler etc...

# On said systems, running PCH will result in a build fail
# with the following error message:
#   one or more PCH files were found, but they were invalid
# See https://bugzilla.redhat.com/show_bug.cgi?id=1721553
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: June 04, 2023
#  Copyright (C) 2022 - 2024 by Trevor SANDY
#
# sample commands [from application root - e.g. lpub3d/]:
# source builds/linux/obs/alldeps/LcLibPCH.sh
# export LC_DIR="$PWD/lclib" && \
# chmod a+x builds/linux/obs/alldeps/LcLibPCH.sh && \
# ./builds/linux/obs/alldeps/LcLibPCH.sh
set +x

# Start message
echo && echo "   Start LcLibPCH.sh execution at $PWD..."

# Set lclib dir
LP3D_LC_DIR=${LC_DIR:-$(echo "$PWD/lclib")}

echo && echo "   LP3D_LC_DIR........[${LP3D_LC_DIR}]"

echo && echo "   Remove precompile header directives from lcLib project file"
echo "   --------------------------------------"
COUNTER=0
LP3D_FILE="${LP3D_LC_DIR}/lclib.pro"
if [ -f ${LP3D_FILE} -a -r ${LP3D_FILE} ]
then
  sed -i -e "/^    CONFIG += precompile_header$/d" \
         -e "/^    PRECOMPILED_HEADER.*$/d" "${LP3D_FILE}" && \
  echo "   $((COUNTER += 1)). Removed precompile_header from [$LP3D_FILE]" || \
  echo "   ERROR: Could not apply update to ${LP3D_FILE}"
else
  echo "   ERROR: Could not update ${LP3D_FILE}"
fi

echo && echo "   Add lc_global.h to lcLib headers"
echo "   --------------------------------------"
COUNTER=0
for LP3D_FILE in $(find ${LP3D_LC_DIR} -not -path "${LP3D_LC_DIR}/common/lc_global.h" -type f -name "*.h")
do
  if [ -f ${LP3D_FILE} -a -r ${LP3D_FILE} ]
  then
    sed -i "s/^#pragma once$/#pragma once\n\n#include \"lc_global.h\"/" "${LP3D_FILE}" && \
    echo "   $((COUNTER += 1)). Add lc_global.h to ${LP3D_FILE}" || \
    echo "   $((COUNTER += 1)).ERROR: Could not add lc_global.h to ${LP3D_FILE}"
  else
    echo "   ERROR: Could not update ${LP3D_FILE}. File was not found."
  fi
done

echo && echo "   Remove lc_global.h from lcLib source files"
echo "   --------------------------------------"
COUNTER=0
for LP3D_FILE in $(find ${LP3D_LC_DIR} -not -path "${LP3D_LC_DIR}/qt/system.cpp" -type f -name "*.cpp")
do
  if [ -f ${LP3D_FILE} -a -r ${LP3D_FILE} ]
  then
    sed -i "/#include \"lc_global.h\"/d" "${LP3D_FILE}" && \
    echo "   $((COUNTER += 1)). Removed lc_global.h from [$LP3D_FILE]" || \
    echo "   $((COUNTER += 1)).ERROR: Could not remove lc_global.h from ${LP3D_FILE}"
  else
    echo "   ERROR: Could not update ${LP3D_FILE}. File was not found."
  fi
done

# End message
echo "LcLibPCH.sh execution finshed." && echo
set -x
