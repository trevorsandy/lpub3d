#!/bin/bash
# Trevor SANDY
# Last Update June 03, 2021
# Copyright (c) 2021 by Trevor SANDY
#
# This script is run from a Docker container call
# See builds/utilities/ci/github/multiarch-build.sh
#
# Note the container must mount the a workspace volume path to '/ws' and
# an output volume path to '/out'
# -v "${repo_root}":/ws
# -v "$(readlink -f out/)":/out
#
# To Run:
# /bin/bash -xc "chmod a+x builds/utilities/ci/github/build-binaries-and-appimage.sh && builds/utilities/ci/github/build-binaries-and-appimage.sh"

set -e
set -x
set -o functrace

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

Info () {
  f="${0##*/}"; f="${f%.*}"
  echo "-${f}: ${*}" >&2
}

Error () {
  Info "ERROR - $*" >&2
}

# Grab the script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# build in a temporary directory, use RAM disk if possible
if [ -d /dev/shm ] && mount | grep /dev/shm | grep -v -q noexec; then
  TEMP_BASE=/dev/shm
elif [ -d /docker-ramdisk ]; then
  TEMP_BASE=/docker-ramdisk
else
  TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" build-XXXXXX)"

finish () {
  if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
  fi
  FinishElapsedTime
}

trap finish EXIT

# Copy source to build directory
cd $BUILD_DIR && mv -f ~/* .

Info "Start $ME execution from $PWD..."

export LP3D_DIST_DIR_PATH=${LP3D_DIST_DIR_PATH:-/dist}
export LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR:-lpub3d_linux_3rdparty}
export LPUB3D=${LPUB3D:-lpub3d}
export LP3D_VER_LDGLITE=ldglite-1.3
export LP3D_VER_LDVIEW=ldview-4.4
export LP3D_VER_POVRAY=lpub3d_trace_cui-3.8
export LP3D_LDRAW_DIR=${LP3D_LDRAW_DIR:-/dist/ldraw}
export CI=${CI:-true}
export GITHUB=${GITHUB:-true}

# Special case, using '/ws instead of 'BUILD_DIR' to acccess .git folder
export _PRO_FILE_PWD_=/ws/mainApp

# Source update_config_files.sh"
source builds/utilities/update-config-files.sh

# Copy files updated by update-config-files.sh
cp -f /ws/builds/utilities/version.info builds/utilities/
cp -f /ws/mainApp/docs/RELEASE_NOTES.html mainApp/docs/
cp -f /ws/mainApp/docs/README.txt mainApp/docs/
cp -f /ws/mainApp/lpub3d.desktop mainApp/
cp -f /ws/mainApp/lpub3d.appdata.xml mainApp/
cp -f /ws/mainApp/docs/lpub3d24.1 mainApp/docs/
cp -f /ws/builds/linux/obs/debian/changelog builds/linux/obs/debian/
cp -f /ws/builds/linux/obs/PKGBUILD builds/linux/obs/
cp -f /ws/builds/linux/obs/lpub3d.spec builds/linux/obs/
cp -f /ws/builds/linux/obs/debian/lpub3d.dsc builds/linux/obs/debian/
cp -f /ws/builds/linux/obs/debian/rules builds/linux/obs/debian/
cp -f /ws/gitversion.pri .

# Trigger 3rdParty renderers from environment variable flag
[ "$LP3D_BUILD_LDGLITE_VAR" = "true" ] && export LP3D_BUILD_3RD=1 && export LP3D_BUILD_LDGLITE=1 \
&& Info "'Build LDGLite' detected in environment variable." || true
[ "$LP3D_BUILD_LDVIEW_VAR" = "true" ] && export LP3D_BUILD_3RD=1 && export LP3D_BUILD_LDVIEW=1 \
&& Info "'Build LDView' detected in environment variable." || true
[ "$LP3D_BUILD_POVRAY_VAR" = "true" ] && export LP3D_BUILD_3RD=1 && export LP3D_BUILD_POVRAY=1 \
&& Info "'Build POV-Ray' detected in environment variable." || true

# Setup ldraw parts library directory
if [ ! -d "$LP3D_LDRAW_DIR" ]; then
  mkdir -p "$LP3D_LDRAW_DIR";
  if test -d "$LP3D_LDRAW_DIR"; then
    Info "Created LDraw library $LP3D_LDRAW_DIR"
  fi
else
  Info "Using cached LDraw library $LP3D_LDRAW_DIR"
fi

# Download LDraw library archive files
Info "Download AppImage archive libraries...";
[[ ! -L "$LP3D_DIST_DIR_PATH" && ! -d "${LP3D_DIST_DIR_PATH}" ]] && mkdir -p "${LP3D_DIST_DIR_PATH}" || true
if [ "${GITHUB}" = "true" ]; then
  wget http://www.ldraw.org/library/unofficial/ldrawunf.zip -O ${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip
fi
if [ ! -f "${LP3D_DIST_DIR_PATH}/complete.zip" ]; then
   wget http://www.ldraw.org/library/updates/complete.zip -O ${LP3D_DIST_DIR_PATH}/complete.zip
fi
if [ ! -f "${LP3D_DIST_DIR_PATH}/tenteparts.zip" ]; then
   wget https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O ${LP3D_DIST_DIR_PATH}/tenteparts.zip
fi
if [ ! -f "${LP3D_DIST_DIR_PATH}/vexiqparts.zip" ]; then
   wget https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O ${LP3D_DIST_DIR_PATH}/vexiqparts.zip
fi

# Manage 3rd party distribution cache - deleting a component will trigger that component's rebuild
if [ -n "$LP3D_BUILD_3RD" ]; then
  LP3D_LDGLITE_DIR=${LP3D_DIST_DIR_PATH}/${LP3D_VER_LDGLITE}
  LP3D_LDVIEW_DIR=${LP3D_DIST_DIR_PATH}/${LP3D_VER_LDVIEW}
  LP3D_POVRAY_DIR=${LP3D_DIST_DIR_PATH}/${LP3D_VER_POVRAY}
  if [[ "$LP3D_BUILD_LDGLITE" = 1 && -d "$LP3D_LDGLITE_DIR" ]]; then
    sudo rm -rf "$LP3D_LDGLITE_DIR" && Info "cached $LP3D_LDGLITE_DIR deleted"
  fi
  if [[ "$LP3D_BUILD_LDVIEW" = 1 && -d "$LP3D_LDVIEW_DIR" ]]; then
    sudo rm -rf "$LP3D_LDVIEW_DIR" && Info "cached $LP3D_LDVIEW_DIR deleted"
  fi
  if [[ "$LP3D_BUILD_POVRAY" = 1 && -d "$LP3D_POVRAY_DIR" ]]; then
    sudo rm -rf "$LP3D_POVRAY_DIR" && Info "cached $LP3D_POVRAY_DIR deleted"
  fi
fi

# List global and local 'LP3D_*' environment variables - use 'env' for 'exported' variables
set +x
Info && Info "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do Info $line=${!line};done
set -x

# Export renderer build variables and set renderer builds output path
Info "Build LPub3D renderers - LDGLite, LDView, POV-Ray"
export WD=$PWD
export OBS=${OBS:-false}
export LOG_PATH=${LOG_PATH:-/out}
export NO_DEPS=${NO_DEPS:-true}
export NO_CLEANUP=${NO_CLEANUP:-true}
export LDRAWDIR=${LP3D_LDRAW_DIR}
export LDRAWDIR_ROOT=${LP3D_DIST_DIR_PATH}
if [ ! -d "${WD}/${LP3D_3RD_DIST_DIR}" ]; then
  ln -sf "$LP3D_DIST_DIR_PATH" "${WD}/${LP3D_3RD_DIST_DIR}"
  if [ -d "${WD}/${LP3D_3RD_DIST_DIR}" ]; then
    Info "$LP3D_DIST_DIR_PATH linked to ${WD}/${LP3D_3RD_DIST_DIR}"
  else
    Error "$LP3D_DIST_DIR_PATH failed to link to ${WD}/${LP3D_3RD_DIST_DIR}"
    exit 4
  fi
else
  Info "Using cached 3rd Party repository ${WD}/${LP3D_3RD_DIST_DIR}"
fi

# Build LPub3D renderers - LDGLite, LDView, POV-Ray
chmod a+x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
#Info "Renderer Files:" && find ${LP3D_DIST_DIR_PATH} -type f && Info;

# Set Appimage build path
export AppImageBuildPath=$(mkdir -p AppDir && readlink -f AppDir/)

# Build LPub3D
if [[ ! -d "/dist/AppDir" || -z "$(ls -A /dist/AppDir)" ]]; then
  if [ -z "$BUILD_CPUs" ]; then
    if [ "$CI" != "true" ]; then
      BUILD_CPUs="$(nproc --ignore=2)"
    else
      BUILD_CPUs="$(nproc)"
    fi
  fi

  qmake -v
  qmake -nocache QMAKE_STRIP=: CONFIG+=release CONFIG-=debug_and_release CONFIG+=api
  make
  #make -j$(BUILD_CPUs)
  make INSTALL_ROOT=$AppImageBuildPath install

  # backup build artifacts in case of failure
  if [[ "$CI" != "true" && $? = 0 ]]; then
     cp -ar $AppImageBuildPath /dist/
  fi
else
  Info "LPub3D build artifacts exists - build skipped."
  cp -ar /dist/AppDir/* $AppImageBuildPath/
fi

# Build check
Info "Build check LPub3D bundle..."
export SOURCE_DIR=${PWD}
export LP3D_BUILD_OS=
export LP3D_CHECK_STATUS="--version --app-paths"
export LPUB3D_EXE="${AppImageBuildPath}/usr/bin/lpub3d24"
chmod a+x builds/check/build_checks.sh
./builds/check/build_checks.sh

exit

# ===============================AppImage Module======================================

cd $AppImageBuildPath

# Setup AppImage linuxdeployqt
Info && Info "Setup linuxdeployqt..."
if [ ! -e linuxdeployqt ]; then
  if [ "$ARCH" = "amd64" ]; then
    Info "Insalling linuxdeployqt for amd64 arch..."
    wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
    chmod a+x linuxdeployqt;
  elif [ "$ARCH" = "aarch64" ]; then
    if [ ! -e /dist/AppDir/bin/linuxdeployqt ]; then
      mkdir bin && export WD=$AppImageBuildPath
      export PATH="${WD}/bin":"$PATH"
      Info "Building linuxdeployqt for aarch64 arch..."
      git clone https://github.com/probonopd/linuxdeployqt.git linuxdeployqt-build
      ( cd linuxdeployqt-build && qmake && make && cp -a ./bin/* ../bin )
      if [ -f ./bin/linuxdeployqt ]; then
        [ -f linuxdeployqt ] && rm -f linuxdeployqt
        ln -sf ./bin/linuxdeployqt linuxdeployqt
        echo "linuxdeployqt linked to bin/linuxdeployqt"
      else 
        Error "failed to link linuxdeployqt"
        exit 5
      fi
      Info "Building patchelf..."
      wget https://nixos.org/releases/patchelf/patchelf-0.9/patchelf-0.9.tar.bz2
      tar xf patchelf-0.9.tar.bz2
      ( cd patchelf-0.9/ && ./configure --prefix=${WD} --exec_prefix=${WD} && make && make install )
      Info "Installing appimagetool..."
      wget -c -nv "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-aarch64.AppImage" -O bin/appimagetool
      chmod a+x bin/appimagetool
      # backup AppDir build artifacts in case of failure
      if [[ "$CI" != "true" && $? = 0 ]]; then
         cp -a linuxdeployqt /dist/AppDir/
         cp -a share/ /dist/AppDir/
         cp -a bin/ /dist/AppDir/         
      fi 
    else 
      Info "Linuxdeployqt artifacts exists - build skipped."
      if [ -f ./bin/linuxdeployqt ]; then
        [ -f linuxdeployqt ] && rm -f linuxdeployqt
        ln -sf ./bin/linuxdeployqt linuxdeployqt && \
        echo "linuxdeployqt linked to bin/linuxdeployqt"
      fi
    fi
    if [ -e /usr/bin/lconvert ]; then
      ( cd bin && ln -sf /usr/bin/lconvert lconvert ) && \
      echo "bin/lconvert linked to /usr/bin/lconvert"
    fi
    # Remove magic bytes [KO]:
    # Triggers error:
    # Read of e_ident from /proc/self/exe failed: Bad file descriptor
    # Failed to get fs offset for /proc/self/exe
    sed -i 's|AI\x02|\x00\x00\x00|' bin/appimagetool    
  fi
  Info "Setup linuxdeployqt completed" && Info
fi

# Build AppImage
Info && Info "Build AppImage from AppImageBuildPath: $AppImageBuildPath..."
renderers=$(find $AppImageBuildPath/opt -type f);
for r in $renderers; do executables="$executables -executable=$r" && echo "Set executable $executables"; done;
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH;
export VERSION="$LP3D_VERSION"    # linuxdeployqt uses this for naming the file
./linuxdeployqt ./usr/share/applications/*.desktop $executables -unsupported-allow-new-glibc -bundle-non-qt-libs -verbose=2
./linuxdeployqt ./usr/share/applications/*.desktop -unsupported-allow-new-glibc -appimage -verbose=2
Info && Info "AppImage Dynamic Library Dependencies:" && \
find ./ -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq && Info
AppImageFile=$(ls LPub3D*.AppImage)
if [ -f $AppImageFile ]; then
  Info "AppImage build completed successfully."
  AppImageExtension=${AppImageFile##*-}
  mv -f ${AppImageFile} "${AppImageBuildPath}/LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension}"
  AppImageFile=LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension}
  Info "Creating hash file for $AppImageFile..."
  sha512sum "${AppImageFile}" > "${AppImageFile}.sha512" || \
  Error "Failed to create hash file ${AppImageFile}.sha512"
  Info "Application package....: AppImageFile"
else
  Error "AppImage build failed. $AppImageFile not found."
  exit 6
fi

# Check AppImage build
AppImageFile=$(find $AppImageBuildPath/AppImage -name LPub3D-${LP3D_APP_VERSION_LONG}*.AppImage -type f);
if [ -n "$AppImageFile" ]; then
  export SOURCE_DIR=${PWD}
  export LP3D_BUILD_OS="appimage"
  export LP3D_CHECK_STATUS="--version --app-paths"
  mkdir appImage_Check
  Info "Build check LPub3D.AppImage bundle..."
  cp -f $AppImageFile appImage_Check/ && \
  Info "- $(ls ./appImage_Check/*.AppImage) copied to check folder."
  export LPUB3D_EXE="appImage_Check/LPub3D-${LP3D_APP_VERSION_LONG}-${ARCH}.AppImage"
  source ${SOURCE_DIR}/builds/check/build_checks.sh
  #[ -d "/dist/AppDir" ] && rm -rf /dist/AppDir
else
  Error "AppImage not found, the build check cannot proceed."
  exit 7
fi

# Move AppImage build content to output
mv -f ${AppImageFile}* /out/
mv -f *.log /out/

exit
