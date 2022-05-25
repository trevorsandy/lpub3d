#!/bin/bash
# Trevor SANDY
# Last Update June 29, 2021
# Copyright (c) 2021 by Trevor SANDY
#
# This script is run from a Docker container call
# See builds/utilities/ci/github/linux-multiarch-build.sh
#
# Note the container must mount the following volumes
#   -v <3rd-party apps path>:/out
#   -v <         ldraw path>:/dist
#   -v <        output path>:/ldraw
#
# To Run:
# /bin/bash -xc "chmod a+x builds/linux/CreateAppImage.sh && builds/linux/CreateLinuxPkg.sh"

set -o functrace

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  ME="${ME} for (${LP3D_BASE}-${LP3D_ARCH})"
  [ "${LP3D_APPIMAGE}" = "true" ] && ME="${ME} for (${LP3D_BASE}-${LP3D_ARCH}-appimage)" || :
  echo "${ME} Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

SaveAppImageSetupProgress() {
  # backup AppDir build artifacts in case of failure
  if [[ "$CI" != "true" && $? = 0 ]]; then
    [ -f linuxdeployqt ] && cp -af linuxdeployqt ${LP3D_DIST_DIR_PATH}/AppDir/ || :
    [ -d share ] && cp -af share/ ${LP3D_DIST_DIR_PATH}/AppDir/ || :
    [ -d bin ] && cp -af bin/ ${LP3D_DIST_DIR_PATH}/AppDir/ || :
  fi
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

Info "Start $ME execution from $PWD..."

# build in a temporary directory, use RAM disk if possible
if [ -d /dev/shm ] && mount | grep /dev/shm | grep -v -q noexec; then
  TEMP_BASE=/dev/shm
elif [ -d /docker-ramdisk ]; then
  TEMP_BASE=/docker-ramdisk
else
  TEMP_BASE=/tmp
fi

export BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" build-XXXXXX)"

finish () {
  if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
  fi
  FinishElapsedTime
}

trap finish EXIT

# Move to build directory
cd $BUILD_DIR

export LP3D_DIST_DIR_PATH=${LP3D_DIST_DIR_PATH:-/dist/${LP3D_BASE}_${LP3D_ARCH}}
export LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR:-lpub3d_linux_3rdparty}
export LPUB3D=${LPUB3D:-lpub3d}
export LP3D_VER_LDGLITE=ldglite-1.3
export LP3D_VER_LDVIEW=ldview-4.4
export LP3D_VER_POVRAY=lpub3d_trace_cui-3.8
export LDRAWDIR_ROOT=~
export LDRAWDIR=~/ldraw
export WD=$PWD
export CI=${CI:-true}
export OBS=${OBS:-false}
export GITHUB=${GITHUB:-true}
export DOCKER=${DOCKER:-true}
export LP3D_LOG_PATH=${LP3D_LOG_PATH:-/out}
export LP3D_NO_DEPS=${LP3D_NO_DEPS:-true}
export LP3D_NO_CLEANUP=${LP3D_NO_CLEANUP:-true}

Info "SOURCE DIR.........${LPUB3D}"
Info "BUILD DIR..........${BUILD_DIR}"
Info "BUILD BASE.........${LP3D_BASE}"
Info "BUILD ARCH.........${LP3D_ARCH}"
Info "QEMU...............${LP3D_QEMU}"
Info "DOCKER.............${DOCKER}"
Info "APPIMAGE...........${LP3D_APPIMAGE}"

# If LP3D_QEMU and not AppImage, source package build script
if [[ "${LP3D_APPIMAGE}" = "false" && "${LP3D_QEMU}" = "true" ]]; then
  # copy files from user directory to build directory
  rsync -avr --exclude='ldraw' --exclude='.config' ~/ .
  # set paths and package script
  case ${LP3D_BASE} in
      "ubuntu")
          pkgblddir="debbuild"
          pkgsrcdir="${pkgblddir}/SOURCES"
          pkgscript="CreateDeb.sh"
          ;;
      "fedora")
          pkgblddir="rpmbuild/BUILD"
          pkgsrcdir="rpmbuild/SOURCES"
          pkgscript="CreateRpm.sh"
          ;;
      "archlinux")
          pkgblddir="pkgbuild/src"
          pkgsrcdir="pkgbuild"
          pkgscript="CreatePkg.sh"
          ;;
      *)
      Error "Invalid build base specified"
      exit 2
      ;;
  esac

  # Link LDraw libraries to package source path
  [ ! -f "${pkgsrcdir}/lpub3dldrawunf.zip" ] && \
  (cd ${pkgsrcdir} && cp -af /dist/lpub3dldrawunf.zip .) || \
  Info "${pkgsrcdir}/lpub3dldrawunf.zip exists. Nothing to do."
  [ ! -f "${pkgsrcdir}/complete.zip" ] && \
  (cd ${pkgsrcdir} && cp -af /dist/complete.zip .) || \
  Info "${pkgsrcdir}/complete.zip exists. Nothing to do."
  [ ! -f "${pkgsrcdir}/tenteparts.zip" ] && \
  (cd ${pkgsrcdir} && cp -af /dist/tenteparts.zip .) || \
  Info "${pkgsrcdir}/tenteparts.zip exists. Nothing to do."
  [ ! -f "${pkgsrcdir}/vexiqparts.zip" ] && \
  (cd ${pkgsrcdir} && cp -af /dist/vexiqparts.zip .) || \
  Info "${pkgsrcdir}/vexiqparts.zip exists. Nothing to do."

  # Link lpub3d_linux_3rdparty to package build source path
  LP3D_TD=${PWD}/${pkgblddir}/${LP3D_3RD_DIST_DIR}
  if [ ! -d "${LP3D_TD}" ]; then
    (cd ${PWD}/${pkgblddir} && ln -sf "${LP3D_DIST_DIR_PATH}" "${LP3D_3RD_DIST_DIR}")
    if [ -d "${LP3D_TD}" ]; then
      Info "${LP3D_DIST_DIR_PATH} linked to ${LP3D_TD}"
    else
      Error "${LP3D_DIST_DIR_PATH} failed to link to ${LP3D_TD}"
    fi
  else
    Info "Using cached 3rd Party repository ${LP3D_TD}"
  fi

  # Source package script
  source ${pkgscript}

  # Done so exit.
  exit 0
fi

# ..............AppImage Calls....................#

# Download or copy source
if [ "${TRAVIS}" != "true" ]; then
   Info "Download source to $BUILD_DIR/..."
   git clone https://github.com/trevorsandy/${LPUB3D}.git
   mv -f ./${LPUB3D}/.[!.]* . && mv -f ./${LPUB3D}/* . && rm -rf ./${LPUB3D}
   rsync -avr --exclude='ldraw' --exclude='.config' ~/ .
else
   Info "Copy source to $BUILD_DIR/..."
   cp -rf "../../${LPUB3D}" .
fi

# For Docker build, check if there is a tag after the last commit
if [ "$DOCKER" = "true" ]; then
  # Setup git command
  GIT_CMD="git --git-dir $PWD/.git --work-tree $PWD"
  #1. Get the latest version tag - check across all branches
  BUILD_TAG=$($GIT_CMD describe --tags --match v* $($GIT_CMD rev-list --tags --max-count=1) 2> /dev/null)
  if [ -n "$BUILD_TAG" ]; then
    #2. Get the tag datetime
    BUILD_TAG_TIME=$($GIT_CMD log -1 --format=%ai $BUILD_TAG 2> /dev/null)
    #3. Get the latest commit datetime from the build branch
    GIT_COMMIT_TIME=$($GIT_CMD log -1 --format=%ai 2> /dev/null)
    #4. If tag is newer than commit, check out the tag
    if [ $(date -d "$GIT_COMMIT_TIME" +%s) -lt $(date -d "$BUILD_TAG_TIME" +%s) ]; then
      echo "2a. checking out build tag $BUILD_TAG..."
      $GIT_CMD checkout -qf $BUILD_TAG
    fi
  fi
fi

# Source update_config_files.sh"
Info "Source update_config_files.sh..."
export _PRO_FILE_PWD_=$PWD/mainApp
set +x && source builds/utilities/update-config-files.sh && set -x

# Link lpub3d_linux_3rdparty to package build source path
LP3D_TD=${PWD}/${LP3D_3RD_DIST_DIR}
if [ ! -d "${LP3D_TD}" ]; then
  (ln -sf "${LP3D_DIST_DIR_PATH}" "${LP3D_3RD_DIST_DIR}")
  if [ -d "${LP3D_TD}" ]; then
    Info "${LP3D_DIST_DIR_PATH} linked to ${LP3D_TD}"
  else
    Error "${LP3D_DIST_DIR_PATH} failed to link to ${LP3D_TD}"
    exit 4
  fi
else
  Info "Using cached 3rd Party repository ${LP3D_TD}"
fi

# Download LDraw library archive files if not available
Info "Download AppImage archive libraries..."
[[ ! -L "$/dist" && ! -d "/dist" ]] && mkdir -p "/dist" || :
[ ! -f "/dist/lpub3dldrawunf.zip" ] && \
wget http://www.ldraw.org/library/unofficial/ldrawunf.zip -O /dist/lpub3dldrawunf.zip || :
(cd ${LP3D_DIST_DIR_PATH} && ln -sf /dist/lpub3dldrawunf.zip lpub3dldrawunf.zip)
[ ! -f "/dist/complete.zip" ] && \
wget http://www.ldraw.org/library/updates/complete.zip -O /dist/complete.zip || :
(cd ${LP3D_DIST_DIR_PATH} && ln -sf /dist/complete.zip complete.zip)
[ ! -f "/dist/tenteparts.zip" ] && \
wget https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O /dist/tenteparts.zip || :
(cd ${LP3D_DIST_DIR_PATH} && ln -sf /dist/tenteparts.zip tenteparts.zip)
[ ! -f "/dist/vexiqparts.zip" ] && \
wget https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip -O /dist/vexiqparts.zip || :
(cd ${LP3D_DIST_DIR_PATH} && ln -sf /dist/vexiqparts.zip vexiqparts.zip)

echo "DEBUG ${LP3D_3RD_DIST_DIR} LIST..."
ls -al ${LP3D_3RD_DIST_DIR}/
echo "DEBUG END"

# List global and local 'LP3D_*' environment variables - use 'env' for 'exported' variables
set +x && \
Info && Info "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do Info $line=${!line}; done && \
set -x

# Build LPub3D renderers - LDGLite, LDView, POV-Ray
chmod a+x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh

# Set Appimage build path
export AppImageBuildPath=$(mkdir -p AppDir && readlink -f AppDir/)

# Build LPub3D
if [[ ! -d "${LP3D_DIST_DIR_PATH}/AppDir" || -z "$(ls -A ${LP3D_DIST_DIR_PATH}/AppDir)" ]]; then
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
  make INSTALL_ROOT=${AppImageBuildPath} install

  # backup build artifacts in case of failure
  if [[ "$CI" != "true" && $? = 0 ]]; then
    cp -ar ${AppImageBuildPath} ${LP3D_DIST_DIR_PATH}/
  fi
else
  Info "LPub3D build artifacts exists - build skipped."
  cp -ar ${LP3D_DIST_DIR_PATH}/AppDir/* ${AppImageBuildPath}/
fi

# Build check
if [ -n "$LP3D_PRE_PACKAGE_CHECK" ]; then
  Info "Per-package build check LPub3D bundle..."
  export LP3D_BUILD_OS=
  export SOURCE_DIR=${PWD}
  export LP3D_CHECK_LDD="1"
  export LP3D_CHECK_STATUS="--version --app-paths"
  export LPUB3D_EXE="${AppImageBuildPath}/usr/bin/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
  chmod a+x builds/check/build_checks.sh && ./builds/check/build_checks.sh
fi

# Stop here on compile only
if [ -n "$LP3D_COMPILE_ONLY" ]; then
  exit 0
fi

Info "Entering AppImage build directory: ${AppImageBuildPath}..."
cd ${AppImageBuildPath}

# Setup AppImage linuxdeployqt
Info && Info "Setup linuxdeployqt..."
if [ ! -e linuxdeployqt ]; then
  if [[ "$LP3D_ARCH" = "amd64" || "$LP3D_ARCH" = "x86_64" ]]; then
    Info "Insalling linuxdeployqt for amd64 arch..."
    wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
  elif [[ "$LP3D_ARCH" = "arm64" || "$LP3D_ARCH" = "aarch64" ]]; then
    if [ ! -e ${LP3D_DIST_DIR_PATH}/AppDir/linuxdeployqt ]; then
      [ ! -d "bin" ] && mkdir bin || :
      export WD=${AppImageBuildPath}
      export PATH="${WD}/bin":"$PATH"
      # LinuxDeployQt
      p=LinuxDeployQt
      Info "Building $p for $LP3D_ARCH..."
      git clone https://github.com/probonopd/linuxdeployqt.git $p
      ( cd $p && qmake && make && cp -a ./bin/* ../ ) >$p.out 2>&1 && mv $p.out $p.ok
      if [[ -f $p.ok && -f ./linuxdeployqt ]]; then
        rm -f $p.ok
        Info "linuxdeployqt copied to ${AppImageBuildPath}"
      else
        Error $p FAILED
        tail -80 $p.out
        exit 5
      fi
      SaveAppImageSetupProgress
      # PatchELF
      p=patchelf-0.9
      Info "Building $p for $LP3D_ARCH......"
      wget https://nixos.org/releases/patchelf/patchelf-0.9/$p.tar.bz2
      tar xf $p.tar.bz2
      ( cd $p && ./configure --prefix=${WD} --exec_prefix=${WD} && make && make install ) >$p.out 2>&1 && mv $p.out $p.ok
      if [ -f $p.ok ]; then
        rm -f $p.ok
        Info $p done
      else
        Error $p FAILED
        tail -80 $p.out
        exit 5
      fi
      SaveAppImageSetupProgress
      # AppImageTool
      p=appimagetool
      Info "Installing $p for $LP3D_ARCH..."
      [ ! -d "bin" ] && mkdir bin || :
      # prebuilt bin generating exec format error
      # [ -f /dist/$p ] && cp -f /dist/$p ./bin/ || \
      # wget -c -nv "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-aarch64.AppImage" -O bin/${AppImage}
      if [ -f ${LP3D_DIST_DIR_PATH}/$p ]; then
          cp -f ${LP3D_DIST_DIR_PATH}/$p ./bin/
          ( chmod a+x bin/$p && ./bin/$p --appimage-version ) >$p.out 2>&1 && mv $p.out $p.ok
          if [ -f $p.ok ]; then
              cat $p.ok
              Info $p is runnable
          else
              Info Run $p FAILED
              tail -80 $p.out
              # exit 5
          fi
      else
          Error Install $p FAILED - file was not found.
          exit 5
      fi
      SaveAppImageSetupProgress
    else
      Info "Linuxdeployqt artifacts exists - build skipped."
    fi
    p=lconvert
    Info "Linking $p..."
    lcv="$(which $p)"
    if [ -n "$lcv" ]; then
      [ ! -d "bin" ] && mkdir bin || :
      ( ln -sf $lcv $p && \
        cd bin && ln -sf $lcv $p ) >$p.out 2>&1 && mv $p.out $p.ok
      if [ -f $p.ok ]; then
        rm -f $p.ok
        Info  "bin/$p linked to $lcv"
        Info  "./$p linked to $lcv" || :
        Info Link $p done
      else
        Error Link $p FAILED
        tail -80 $p.out
        exit 5
      fi
    fi
    SaveAppImageSetupProgress
  fi
  chmod a+x linuxdeployqt && ./linuxdeployqt -version
  Info "Setup linuxdeployqt completed" && Info
fi

# Build AppImage
Info && Info "Building AppImage from AppImageBuildPath: ${AppImageBuildPath}..."
renderers=$(find ${AppImageBuildPath}/opt -type f);
for r in $renderers; do executables="$executables -executable=$r" && Info "Set executable $executables"; done;
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH;
export VERSION="$LP3D_VERSION"    # linuxdeployqt uses this for naming the file
./linuxdeployqt ${AppImageBuildPath}/usr/share/applications/*.desktop $executables -bundle-non-qt-libs -verbose=2
./linuxdeployqt ${AppImageBuildPath}/usr/share/applications/*.desktop -appimage -verbose=2
Info && Info "AppImage Dynamic Library Dependencies:" && \
find ${AppImageBuildPath} -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq && Info
AppImage=$(ls LPub3D*.AppImage)
if [ -f "${AppImage}" ]; then
  chmod a+x ${AppImage}
  if [[ ("$LP3D_ARCH" = "arm64" || "$LP3D_ARCH" = "aarch64") && -z "$LP3D_AI_MAGIC_BYTES" ]]; then
    # Patch AppImage magic bytes
    p=AppImagePatch
    AppImageMagicBytes="$(hexdump -Cv ${AppImage} | head -n 1 | grep '41 49 02 00')"
    if [ -n "${AppImageMagicBytes}" ]; then
      ( ${AppImage} --appimage-version ) >$p.out 2>&1 && mv $p.out $p.ok
      if [ ! -f $p.ok ]; then
        Info "AppImage magic bytes: ${AppImageMagicBytes}"
        Info "Patching AppImage magic bytes..."
        dd if=/dev/zero of="${AppImage}" bs=1 count=3 seek=8 conv=notrunc
        if [ -z "$(hexdump -Cv ${AppImage} | head -n 1 | grep '41 49 02 00')" ]; then
          Info "Magic bytes patched: $(hexdump -Cv ${AppImage} | head -n 1)"
          ( ${AppImage} --appimage-version ) >$p.out 2>&1 && mv $p.out $p.ok
          if [ -f $p.ok ]; then
            cat $p.ok
            rm -f $p.ok
            Info "${AppImage} is runnable"
          else
            Error "Run AppImage FAILED"
            tail -80 $p.out
            exit 7
          fi
        else
          Error "Patch AppImage magic bytes FAILED"
          hexdump -Cv ${AppImage} | head -n 3
          exit 7
        fi
      else
        cat $p.ok
        rm -f $p.ok
        Info "${AppImage} is runnable"
      fi
    else
      Info "Magic bytes 'AI' not found in AppImage"
      hexdump -Cv ${AppImage} | head -n 3
      ( ${AppImage} --appimage-version ) >$p.out 2>&1 && mv $p.out $p.ok
      if [ -f $p.ok ]; then
        cat $p.ok
        rm -f $p.ok
        Info "${AppImage} is runnable"
      else
        Error "Run AppImage FAILED"
        tail -80 $p.out
        exit 7
      fi      
    fi
  fi
  AppImageExtension=${AppImage##*-}
  mv -f ${AppImage} "LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension}"
  AppImage=LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension}
  Info "Creating hash file for ${AppImage}..."
  sha512sum "${AppImage}" > "${AppImage}.sha512" || \
  Error "Failed to create hash file ${AppImage}.sha512"
  Info "Application package....: ${AppImage}"
  echo "Package path...........: ${PWD}/${AppImage}"
  if [ -f "${AppImage}.sha512" ]; then
    Info "AppImage build completed successfully."
  else
    Error "AppImage build completed but the .sha512 file was not found."
  fi
else
  Error "AppImage build FAILED. ${AppImage} not found."
  exit 8
fi

# Check AppImage build
AppImage=$(find ${AppImageBuildPath}/ -name LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension} -type f)
if [ -f "${AppImage}" ]; then
  cd ${BUILD_DIR}
  export SOURCE_DIR=$PWD
  export LP3D_BUILD_OS="appimage"
  export LP3D_CHECK_STATUS="--version --app-paths"
  Info "Build check LPub3D.AppImage bundle..."
  mkdir -p appImage_Check && cp -f ${AppImage} appImage_Check/ && \
  Info "$(ls ./appImage_Check/*.AppImage) copied to check folder."
  export LPUB3D_EXE="appImage_Check/LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension}"
  set +x && source ${SOURCE_DIR}/builds/check/build_checks.sh && set -x
else
  Error "AppImage not found, the build check cannot proceed."
  exit 8
fi

# Move AppImage build content to output
Info "Moving AppImage build assets and logs to output folder..."
mv -f ${AppImage}* /out/ 2>/dev/null || :
mv -f ${AppImageBuildPath}/*.log /out/ 2>/dev/null || :
mv -f ${BUILD_DIR}/*.log /out/ 2>/dev/null || :
mv -f ./*.log /out/ 2>/dev/null || :
mv -f ~/*.log /out/ 2>/dev/null || :

exit 0
