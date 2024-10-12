#!/bin/bash
#
# Extract and patch Qt5 library
# This script is used exclusively on Open Build Service
# for platforms that do not have Qt5 - e.g. CentOS 6, RHEL 6/7...
#
# This script is sourced to extract and bin-patch the Qt5 assets.
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: November 11, 2022
#  Copyright (C) 2017 - 2024 by Trevor SANDY
#
# sample command [call from root build directory - e.g. lpub3d/]:
# source builds/linux/obs/alldeps/GetQt5Libs.sh
set +x

# Start message
echo && echo "Start GetQt5Libs.sh execution at $PWD..."

# Set working directory and qt dir path
LP3D_OBS_WD=$PWD && echo "Entering Qt5 working directory..."
LP3D_QT_WD=$(cd ../ && echo $PWD)
LP3D_QT_VER="5.9.3"
LP3D_QT5_DIR="qt5/${LP3D_QT_VER}/gcc_64"
LP3D_QT5_DIR_PATH=$LP3D_QT_WD/$LP3D_QT5_DIR

echo
echo "OBS_WORK_DIR.........${LP3D_OBS_WD}"
echo "QT_WORK_DIR..........${LP3D_QT_WD}"
echo "LP3D_QT_VER..........${LP3D_QT_VER}"
echo "LP3D_QT5_DIR_PATH....$LP3D_QT5_DIR_PATH"
echo

# Extract Qt library
cd $LP3D_QT_WD && [ ! -d "$LP3D_QT5_DIR" ] && \
echo "Extracting tarball qt5-${LP3D_QT_VER}-gcc_64-el.tar.gz..." && \
tar -xzf qt5-${LP3D_QT_VER}-gcc_64-el.tar.gz && \
echo "Extracted tarball qt5-${LP3D_QT_VER}-gcc_64-el.tar.gz" || :

# Set QtBinPatcher command options
[ "$RPM_STAGE" = "build" ] && \
patchopts="--verbose --nobackup" && \
echo "Using QtBinPatcher options: $patchopts" || :

# Run QtBinPatcher - export QT5_BIN path
pushd $LP3D_QT5_DIR/bin
  [ -f "qmake" ] && ./qtbinpatcher $patchopts && \
  export LP3D_QT5_BIN=$PWD && export LP3D_QT5_DIR_PATH || \
  echo "ERROR - Could not run QtBinPatcher"
popd

# Update ld_library_path
echo "Prepending LD_LIBRARY_PATH with $LP3D_QT5_DIR_PATH/bin" && \
export LD_LIBRARY_PATH=$LP3D_QT5_DIR_PATH/bin:$LD_LIBRARY_PATH

# Restore working directory
cd $LP3D_OBS_WD && echo "Return to working directory: $LP3D_OBS_WD"

# End message
echo "GetQt5Libs.sh execution finshed." && echo
set -x
