#!/bin/bash
#
# Extract local libraries for OSMesa, LLVM, OpenEXR and libDRM
#
# This script is sourced to extract local library assets.
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: September 17, 2024
#  Copyright (C) 2017 - 2024 by Trevor SANDY
#
# sample command [call from root build directory - e.g. lpub3d/]:
# source builds/linux/obs/alldeps/GetLocalLibs.sh
set +x

# Start message
echo && echo "Start GetLocalLibs.sh execution at $PWD..."

# Set working directory and qt dir path
LP3D_OBS_WD=$PWD && echo "Entering working directory [$LP3D_OBS_WD]..."
LP3D_LL_WD=$(cd ../ && echo $PWD) && cd $LP3D_LL_WD

# Set user and etc paths
LP3D_LL_USR=$LP3D_LL_WD/usr
LP3D_LL_ETC=$LP3D_LL_WD/etc
LP3D_LL_BIN=$LP3D_LL_USR/bin

echo
echo "LOCAL_LIB_WORK_DIR...${LP3D_LL_WD}"
echo "LOCAL_LIB_USR_DIR....${LP3D_LL_USR}"
echo "LOCAL_LIB_ETC_DIR....${LP3D_LL_ETC}"
echo "LOCAL_LIB_BIN_DIR....${LP3D_LL_BIN}"
[ -n "$skip_local_POVRay_libs" ] && \
echo "SKIP_POVRAY_LIBS.....Yes" || \
echo "SKIP_POVRAY_LIBS.....No"
echo

# Extract library folders
if [ "$RPM_STAGE" = "build" ]
then
  # Extact tarball
  [ ! -f "mesa-libOSMesa-17.0.1-6.20170307.el7.x86_64.rpm" ] && \
  echo "Extracting locallibs.el.x86_64.tar.gz..." && \
  tar -xzvf locallibs.el.x86_64.tar.gz && \
  echo "Extracted locallibs.el.x86_64.tar.gz" && echo || true

  # Skip local OpebEXR ibrary files
  if [ -n "$skip_local_POVRay_libs" ]
  then
    for file in $(find . -type f \
                  -name '*ilmbase-*.el7.x86_64.rpm' -o \
                  -name '*libdrm-*.el7.x86_64.rpm' -o \
                  -name '*OpenEXR-*.el7.x86_64.rpm' \
                 )
    do
      rm -rf $file && echo "Removed local OpebEXR lib $file" || \
      echo "ERROR - local OpebEXR lib $file was not removed"
    done || echo "ERROR - local POVRay library files were not skipped"
    echo
  fi

  # Extract rpm libraries into library folder(s)
  [ -f "mesa-libOSMesa-17.0.1-6.20170307.el7.x86_64.rpm" ] && \
  for file in $(find . -type f -name '*.el7.x86_64.rpm')
  do
    rpm2cpio $file | cpio -idm && echo "Extracted local lib rpm $file..." || \
    echo "ERROR - local lib rpm $file was not extracted"
  done || echo "ERROR - locallibs.el.x86_64.tar.gz was not extracted properly"
  echo

  # Export usr base paths - consumed by qmake install
  [ -d $LP3D_LL_USR ] && export LP3D_LL_USR || \
  echo "ERROR - $LP3D_LL_USR does not exist"
  [ -d $LP3D_LL_ETC ] && export LP3D_LL_ETC || \
  echo "ERROR - $LP3D_LL_ETC does not exist"

  # Create symlink for llvm-config-64
  pushd $LP3D_LL_BIN
    [ -f llvm-config-64 ] && \
    ln -s llvm-config-64 llvm-config && \
    [ -f llvm-config ] && export LP3D_LL_BIN && \
    echo "Created symlink $LP3D_LL_USR/bin/llvm-config" || \
    echo "ERROR - unable to create symlink $LP3D_LL_USR/bin/llvm-config"
    echo
  popd

  # Generate symlinks if not exist, else list libdir files and symlinks
  # given libx.so.0.0.0, generate libx.so.0.0, libx.so.0, and libx.so
  pushd $LP3D_LL_USR/lib64
    [ -z "$(find . -type l)" ] && \
    creatingLinks="Creating symlink for local libs..." && echo "$creatingLinks" && \
    for file in $(find . -type f -name 'lib*.so.*')
    do
      echo "File:    ${file##*/}" && shortlib=$file && basename=$file
      while extn=$(echo $shortlib | sed -n '/\.[0-9][0-9]*$/s/.*\(\.[0-9][0-9]*\)$/\1/p')
        [ -n "$extn" ]
      do
        shortlib=$(basename $shortlib $extn)
        ln -fs $basename $shortlib
        basename=$shortlib
        echo "Symlink: $basename $shortlib"
      done
    done || true
    # for some reason .so symlinks are not automatically created for these 2 libs...
    [[ -f libGLU.so.1.3.1 && ! -f libGLU.so ]] && \
    ln -fs libGLU.so.1.3.1 libGLU.so && \
    [ -f libGLU.so ] && echo "Created symlink libGLU.so" || \
    echo "ERROR - unable to create symlink libGLU.so"
    [[ -f libglapi.so.0.0.0 && ! -f libglapi.so ]] && \
    ln -fs libglapi.so.0.0.0 libglapi.so && \
    [ -f libglapi.so ] && echo "Created symlink libglapi.so" || \
    echo "ERROR - unable to create symlink libglapi.so"
    # show list of files and symlinks
    [ -z "$creatingLinks" ] && \
    echo "Library file and symlink list..." && \
    find -name 'lib*.so*' || true
    echo
  popd

  # Update pc files...
  pushd ${LP3D_LL_USR}/lib64/pkgconfig
    echo "Update local libs .pc files..." && \
    for file in $(find . -type f)
    do
      sed -i "s,/usr,${LP3D_LL_USR},g" $file
    done
    echo
  popd
fi

# Restore working directory
cd $LP3D_OBS_WD && echo "Return to working directory: $LP3D_OBS_WD"

# End message
echo "GetLocalLibs.sh execution finshed." && echo
set -x
