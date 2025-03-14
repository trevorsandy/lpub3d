#!/bin/bash
# Trevor SANDY
# Last Update March 13, 2025
# Copyright (C) 2022 - 2025 by Trevor SANDY
#
# This script is run from a Docker container call
# See builds/utilities/ci/github/linux-build.sh
#
# Note the container must mount the following volumes
#   -v <3rd-party apps path>:/out
#   -v <         ldraw path>:/dist
#   -v <        output path>:/ldraw
#
# To Run:
# /bin/bash -xc "chmod a+x builds/linux/CreateLinux.sh && builds/linux/CreateLinux.sh"

set -o functrace

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: (($SECONDS / 3600))hrs ((($SECONDS / 60) % 60))min (($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  ME="${ME} for (${LP3D_BASE}-${LP3D_ARCH})"
  [ "${LP3D_APPIMAGE}" = "true" ] && ME="${ME} for (${LP3D_BASE}-${LP3D_ARCH}-appimage)" || :
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "${ME} Verification Finished!"
  else
    echo "${ME} Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

SaveAppImageSetupProgress() {
  # backup AppDir build artifacts in case of failure
  if [[ "${CI}" != "true" && $? = 0 ]]; then
    mkdir -p ${LP3D_DIST_DIR_PATH}/AppDir/tools
    [ -f "${AppDirBuildPath}/linuxdeployqt" ] && \
    cp -af ${AppDirBuildPath}/linuxdeployqt ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
    if [ -n "${LP3D_AI_BUILD_TOOLS}" ]; then
      [ -d "${WD}/bin" ] && cp -af bin/ ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
      [ -d "${WD}/share" ] && cp -af share/ ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
      [ -f "${WD}/patchelf-0.9.tar.bz2" ] && \
      cp -af patchelf-0.9.tar.bz2 ${LP3D_DIST_DIR_PATH}/AppDir/tools/ || :
    fi
  fi
}

Info () {
  f="${0##*/}"; f="${f%.*}"
  echo "-${f}: ${*}" >&2
}

Error () {
  Info "ERROR - $*" >&2
}

# Format name and set WRITE_LOG - SOURCED if $1 is empty
ME="CreateLinuxMulitArch"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

# automatic logging
[ -d "/out" ] && LP3D_LOG_PATH=/out
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
if [ "${WRITE_LOG}" = "true" ]; then
  f="${0##*/}"; f="${f%.*}"; [ -n "${LP3D_ARCH}" ] && f="${f}-${LP3D_ARCH}" || f="${f}-amd64"
  [ "${LP3D_APPIMAGE}" = "true" ] && f="${f}-appimage" || :
  f="${LP3D_LOG_PATH}/${f}"
  ext=".log"
  if [[ -e "$f$ext" ]] ; then
    i=1
    f="${f%.*}";
    while [[ -e "${f}_${i}${ext}" ]]; do
      let i++
    done
    f="${f}_${i}${ext}"
  else
    f="${f}${ext}"
  fi
  # output log file
  LOG="$f"
  exec > >(tee -a ${LOG} )
  exec 2> >(tee -a ${LOG} >&2)
fi

export WRITE_LOG
export LP3D_LOG_PATH

Info "Start $ME execution from $PWD..."

# build in a temporary directory, use RAM disk if possible
if [ -d /dev/shm ] && mount | grep /dev/shm | grep -v -q noexec; then
  TEMP_BASE=/dev/shm
elif [ -d /docker-ramdisk ]; then
  TEMP_BASE=/docker-ramdisk
else
  TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "${TEMP_BASE}" build-XXXXXX)"

export BUILD_DIR=$BUILD_DIR

finish () {
  if [ -d "${BUILD_DIR}" ]; then
    rm -rf "${BUILD_DIR}"
  fi
  FinishElapsedTime
}

trap finish EXIT

# Move to build directory
cd $BUILD_DIR || exit 1

export LP3D_DIST_DIR_PATH=${LP3D_DIST_DIR_PATH:-/dist/${LP3D_BASE}_${LP3D_ARCH}}
export LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR:-lpub3d_linux_3rdparty}
export LPUB3D=${LPUB3D:-lpub3d}
export LP3D_VER_LDGLITE=ldglite-1.3
export LP3D_VER_LDVIEW=ldview-4.5
export LP3D_VER_POVRAY=lpub3d_trace_cui-3.8
export LDRAWDIR_ROOT=~
export LDRAWDIR=~/ldraw
export WD=$PWD
export CI=${CI:-true}
export OBS=${OBS:-false}
export GITHUB=${GITHUB:-true}
export DOCKER=${DOCKER:-true}
export LP3D_QEMU=${LP3D_QEMU:-false}
export LP3D_NO_DEPS=${LP3D_NO_DEPS:-true}
export LP3D_LOG_PATH=${LP3D_LOG_PATH:-/out}
export LP3D_NO_CLEANUP=${LP3D_NO_CLEANUP:-true}

[ -n "${BUILD_OPT}" ] && Info "BUILD OPTION.......${BUILD_OPT}" || :
[ -n "${LPUB3D}" ] && Info "SOURCE DIR.........${LPUB3D}" || :
[ -n "${BUILD_DIR}" ] && Info "BUILD DIR..........${BUILD_DIR}" || :
[ -n "${LP3D_BASE}" ] && Info "BUILD BASE.........${LP3D_BASE}" || :
[ -n "${LP3D_ARCH}" ] && Info "BUILD ARCH.........${LP3D_ARCH}" || :
[ -n "${LP3D_QEMU}" ] && Info "QEMU...............${LP3D_QEMU}" || :
[ -n "${CI}" ] && Info "CI.................${CI}" || :
[ -n "${GITHUB}" ] && Info "GITHUB.............${GITHUB}" || :
[ -n "${LP3D_APPIMAGE}" ] && Info "APPIMAGE...........${LP3D_APPIMAGE}" || :
if [ "${LP3D_QEMU}" = "true" ]; then
Info "PRE-PACKAGE CHECK..$([ -n "${LP3D_PRE_PACKAGE_CHECK}" ] && echo "true" || echo "false")"
fi
if [ "${LP3D_APPIMAGE}" = "true" ]; then
Info "BUILD AI TOOLS.....$([ -n "${LP3D_AI_BUILD_TOOLS}" ] && echo "true" || echo "false")"
Info "PATCH MAGIC_BYTES..$([ -n "${LP3D_AI_MAGIC_BYTES}" ] && echo "true" || echo "false")"
Info "EXTRACT AI PAYLOAD.$([ -n "${LP3D_AI_EXTRACT_PAYLOAD}" ] && echo "true" || echo "false")"
fi

# Download LDraw library archive files if not available
Info && Info "Checking LDraw archive libraries..."
[[ ! -L "$/dist" && ! -d "/dist" ]] && mkdir -p "/dist" || :
[ ! -f "/dist/lpub3dldrawunf.zip" ] && \
wget https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/lpub3dldrawunf.zip -O /dist/lpub3dldrawunf.zip || :
[ ! -f "/dist/complete.zip" ] && \
wget https://library.ldraw.org/library/updates/complete.zip -O /dist/complete.zip || :
[ ! -f "/dist/tenteparts.zip" ] && \
wget https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O /dist/tenteparts.zip || :
[ ! -f "/dist/vexiqparts.zip" ] && \
wget https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip -O /dist/vexiqparts.zip || :

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

  # Copy LDraw libraries to package source path
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
  LP3D_TD=${WD}/${pkgblddir}/${LP3D_3RD_DIST_DIR}
  if [ ! -d "${LP3D_TD}" ]; then
    (cd ${WD}/${pkgblddir} && ln -sf "${LP3D_DIST_DIR_PATH}" "${LP3D_3RD_DIST_DIR}")
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

# ............Local Build Calls...................#

# Copy or download source
if [ "${TRAVIS}" != "true" ]; then
   if [ -d "/in" ]; then
     Info "Copy source to ${BUILD_DIR}/..."
     cp -rf /in/. .
   else
     Info "Download source to ${BUILD_DIR}/..."
     git clone https://github.com/trevorsandy/${LPUB3D}.git
     mv -f ./${LPUB3D}/.[!.]* . && mv -f ./${LPUB3D}/* . && rm -rf ./${LPUB3D}
   fi
else
   Info "Copy source to ${BUILD_DIR}/..."
   cp -rf "../../${LPUB3D}" .
fi

# For Docker build, check if there is a tag after the last commit
if [ "${DOCKER}" = "true" ]; then
  # Setup git command
  GIT_CMD="git --git-dir ${WD}/.git --work-tree ${WD}"
  # Pull latest
  [ "${CI}" = "true" ] && ${GIT_CMD} pull || :
  #1. Get the latest version tag - check across all branches
  BUILD_TAG="$(${GIT_CMD} describe --tags --match v* "$(${GIT_CMD} rev-list --tags --max-count=1)" 2> /dev/null)"
  if [ -n "${BUILD_TAG}" ]; then
    #2. Get the tag datetime
    BUILD_TAG_TIME=$(${GIT_CMD} log -1 --format=%ai $BUILD_TAG 2> /dev/null)
    #3. Get the latest commit datetime from the build branch
    GIT_COMMIT_TIME=$(${GIT_CMD} log -1 --format=%ai 2> /dev/null)
    #4. If tag is newer than commit, check out the tag
    if [[ "$(date -d "${GIT_COMMIT_TIME}" +%s)" -lt "$(date -d "${BUILD_TAG_TIME}" +%s)" ]]; then
      Info "2a. checking out build tag ${BUILD_TAG}..."
      ${GIT_CMD} checkout -qf ${BUILD_TAG}
    fi
  fi
fi

# Source update_config_files.sh"
Info "Source update_config_files.sh..."
export _PRO_FILE_PWD_=${WD}/mainApp
set +x && source builds/utilities/update-config-files.sh && set -x

# Link lpub3d_linux_3rdparty to package build source path
LP3D_TD=${WD}/${LP3D_3RD_DIST_DIR}
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

# Link and copy locally LDraw library archive files
Info "Link archive libraries to ${LP3D_DIST_DIR_PATH}..."
(cd ${LP3D_DIST_DIR_PATH} && \
ln -sf /dist/lpub3dldrawunf.zip lpub3dldrawunf.zip && \
ln -sf /dist/complete.zip complete.zip && \
ln -sf /dist/tenteparts.zip tenteparts.zip && \
ln -sf /dist/vexiqparts.zip vexiqparts.zip) && \
Info "Copy archive libraries to ${BUILD_DIR}..." && \
cp -af /dist/*.zip .

# List global and local 'LP3D_*' environment variables - use 'env' for 'exported' variables
set +x && \
Info && Info "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do Info $line=${!line}; done && \
set -x

# Build LPub3D renderers - LDGLite, LDView, POV-Ray
chmod a+x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh

# Set application build path
[ ! -d AppDir ] && mkdir -p AppDir || :
AppDirBuildPath=$(readlink -f AppDir/)
export AppDirBuildPath=$AppDirBuildPath

# Build LPub3D
if [[ ! -d "${LP3D_DIST_DIR_PATH}/AppDir/usr" || -z "$(ls -A ${LP3D_DIST_DIR_PATH}/AppDir/usr)" ]]; then
  if [[ "${LP3D_APPIMAGE}" == "false" ]]; then
    case ${LP3D_BASE} in
      "ubuntu")
        distropkg=deb ;;
      "fedora")
        distropkg=rpm ;;
      "archlinux")
        distropkg=pkg ;;
    esac
  else
    distropkg=api
  fi

  # qmake setup
  export QT_SELECT=qt5
  if which qmake-qt5 >/dev/null 2>&1; then
    QMAKE_EXEC=qmake-qt5
  else
    QMAKE_EXEC=qmake
  fi
  ${QMAKE_EXEC} -v

  # build command
  ${QMAKE_EXEC} -nocache QMAKE_STRIP=: CONFIG+=release CONFIG-=debug_and_release CONFIG+=${distropkg}
  make || exit 1
  make INSTALL_ROOT=${AppDirBuildPath} install || exit 1

  # copy AppRun to AppDir
  if [ "${LP3D_APPIMAGE}" = "true" ]; then
    Info "Copy personalized AppRun script to AppDir"
    cp -af builds/linux/obs/alldeps/AppRun ${AppDirBuildPath}
    chmod a+x ${AppDirBuildPath}/AppRun
  fi

  # backup build artifacts in case of failure
  if [[ "${CI}" != "true" && $? = 0 ]]; then
    cp -ar ${AppDirBuildPath} ${LP3D_DIST_DIR_PATH}/
  fi
else
  Info "LPub3D build artifacts exists - build skipped."
  [ -d "${LP3D_DIST_DIR_PATH}/AppDir/usr" ] && \
  cp -ar ${LP3D_DIST_DIR_PATH}/AppDir/usr ${AppDirBuildPath}/ || :
  [ -d "${LP3D_DIST_DIR_PATH}/AppDir/opt" ] && \
  cp -ar ${LP3D_DIST_DIR_PATH}/AppDir/opt ${AppDirBuildPath}/ || :
  if [ -d "${LP3D_DIST_DIR_PATH}/AppDir/tools" ]; then
    [ -f "${LP3D_DIST_DIR_PATH}/AppDir/tools/linuxdeployqt" ] && \
    cp -af ${LP3D_DIST_DIR_PATH}/AppDir/tools/linuxdeployqt ${AppDirBuildPath}/ || :
    if [ -n "${LP3D_AI_BUILD_TOOLS}" ]; then
      [ -d "${LP3D_DIST_DIR_PATH}/AppDir/tools/bin" ] && \
      cp -ar ${LP3D_DIST_DIR_PATH}/AppDir/tools/bin ${WD}/ || :
      [ -d "${LP3D_DIST_DIR_PATH}/AppDir/tools/share" ] && \
      cp -ar ${LP3D_DIST_DIR_PATH}/AppDir/tools/share ${WD}/ || :
      [ -f "${LP3D_DIST_DIR_PATH}/AppDir/tools/patchelf-0.9.tar.bz2" ] && \
      cp -af ${LP3D_DIST_DIR_PATH}/AppDir/tools/patchelf-0.9.tar.bz2 ${WD}/ || :
    fi
  fi
fi

# Build check
# If QEMU, BUILD_OPT will not be 'verify', and if AMD AppImage, check will run from the AppImage distro
if [[ ( ! "${LP3D_APPIMAGE}" == "true" && "${BUILD_OPT}" == "verify" ) || -n "${LP3D_PRE_PACKAGE_CHECK}" ]]; then
  Info "Build check LPub3D bundle..."
  export LP3D_BUILD_OS=
  export SOURCE_DIR=${WD}
  export LP3D_CHECK_LDD="1"
  export LP3D_CHECK_STATUS="--version --app-paths"
  export LPUB3D_EXE="${AppDirBuildPath}/usr/bin/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
  chmod a+x builds/check/build_checks.sh && ./builds/check/build_checks.sh
fi

# Stop here if not building an AppImage
if [ ! "${LP3D_APPIMAGE}" == "true" ]; then
  exit 0
fi

# ..........AppImage Build Calls..................#

# Setup AppImage tools - linuxdeployqt, lconvert
Info && Info "Installing AppImage tools..."
if [[ -z "${LP3D_AI_BUILD_TOOLS}" && ("${LP3D_ARCH}" = "amd64" || "${LP3D_ARCH}" = "x86_64") ]]; then
  cd "${AppDirBuildPath}" || exit 1
  CommandArg=-version
  if [ ! -e linuxdeployqt ]; then
    Info "Insalling linuxdeployqt for ${LP3D_ARCH}..."
    wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
  fi
  ( chmod a+x linuxdeployqt && ./linuxdeployqt ${CommandArg} ) >$p.out 2>&1 && rm -f $p.out
  if [ ! -f $p.out ]; then
    Info "Install linuxdeployqt completed"
  else
    Error Install $p FAILED
    tail -80 $p.out
    exit 5
  fi
  SaveAppImageSetupProgress
elif [[ -n "${LP3D_AI_BUILD_TOOLS}" || "${LP3D_ARCH}" = "arm64" || "${LP3D_ARCH}" = "aarch64" || "${LP3D_QEMU}" = "true" ]]; then
  cd "${WD}/" || exit 1
  [ ! -d bin ] && mkdir bin || :
  export PATH="${WD}/bin":"${PATH}"
  CommandArg=-version
  # LinuxDeployQt
  p=LinuxDeployQt
  if [ ! -e ${AppDirBuildPath}/linuxdeployqt ]; then
    [ -d "$p" ] && rm -rf $p || :
    Info "Building $p for ${LP3D_ARCH} at ${PWD}..."
    git clone https://github.com/probonopd/linuxdeployqt.git $p
    ( cd $p && qmake && make && cp -a ./bin/* ${AppDirBuildPath}/ ) >$p.out 2>&1 && rm -f $p.out
    if [[ ! -f $p.out && -f ${AppDirBuildPath}/linuxdeployqt ]]; then
      Info "linuxdeployqt copied to ${AppDirBuildPath}"
    else
      Error $p FAILED
      tail -80 $p.out
      exit 5
    fi
    SaveAppImageSetupProgress
  fi
  ( cd ${AppDirBuildPath} && chmod a+x linuxdeployqt && ./linuxdeployqt ${CommandArg} ) >$p.out 2>&1 && rm -f $p.out
  if [ ! -f $p.out ]; then
    Info Build $p completed
  else
    Error Build $p FAILED
    tail -80 $p.out
    exit 5
  fi

  # PatchELF
  p=patchelf-0.9
  if [ ! -e bin/patchelf ]; then
    [ -d "$p" ] && rm -rf $p || :
    Info "Building $p for ${LP3D_ARCH}......"
    [ ! -f $p.tar.bz2 ] && \
    wget https://nixos.org/releases/patchelf/patchelf-0.9/$p.tar.bz2 || :
    tar xf $p.tar.bz2
    ( cd $p && ./configure --prefix=${WD} --exec_prefix=${WD} && make && make install ) >$p.out 2>&1 && mv $p.out $p.ok
    if [ ! -f $p.out ]; then
      cat $p.ok
      Info Build $p completed
    else
      Error Build $p FAILED
      tail -80 $p.out
      exit 5
    fi
    SaveAppImageSetupProgress
  else
    Info $p exists. Nothing to do.
  fi

  # AppImageTool
  p=appimagetool
  CommandArg=--appimage-version
  if [ ! -e bin/appimagetool ]; then
    Info "Setting up $p for ${LP3D_ARCH} at ${PWD}..."
    if [ -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$p ]; then
      cp -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$p bin/ && chmod a+x bin/$p
      ( ./bin/$p ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
      formaterror="Exec format error"
      if [ "$(grep -F "${formaterror}" $p.out 2>/dev/null)" ]; then
        echo Format error, command $p ${CommandArg} FAILED
        tail -80 $p.out
        if [ -z "${LP3D_AI_MAGIC_BYTES}" ]; then
          mb="41 49 02 00"
          hd="$(hexdump -Cv bin/$p | head -n 1 | (grep -oE '41 49 02 00'))"
          if [ "${mb}" = "${hd}" ]; then
            echo "$p magic bytes: ${hd}"
            echo "Patching out $p magic bytes..."
            dd if=/dev/zero of="bin/$p" bs=1 count=3 seek=8 conv=notrunc
            if [ -z "$(hexdump -Cv bin/$p | head -n 1 | (grep -oE '41 49 02 00'))" ]; then
              echo "$p magic bytes patched $(hexdump -Cv bin/$p | head -n 1)"
              ( ./bin/$p ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
              if [ -f $p.ok ]; then
                unset runnablecheck
                cat $p.ok >> ${LP3D_LOG_PATH}/$p.ok && rm $p.ok
                echo $p is runnable
              else
                echo Patched magic bytes command $p ${CommandArg} FAILED
                tail -80 $p.out
              fi
            else
              echo Patch $p magic bytes FAILED
              hexdump -Cv bin/$p | head -n 3
              exit 5
            fi
          else
            echo "Magic bytes 'AI' not found in $p"
            hexdump -Cv bin/$p | head -n 3
          fi
        fi
      elif [ -f $p.ok ]; then
        cat $p.ok >> ${LP3D_LOG_PATH}/$p.ok && rm $p.ok
        echo $p is runnable
      else
        Command $p ${CommandArg} FAILED
        tail -80 $p.out
        exit 5
      fi
      [ -f "${LP3D_LOG_PATH}/$p.ok" ] && mv ${LP3D_LOG_PATH}/$p.ok ${LP3D_LOG_PATH}/$p.ok.log || :
      ( [ ! -d AitDir ] && mkdir AitDir || : ; cd AitDir && cp ../bin/$p . && \
      ./$p --appimage-extract ) >$p.out 2>&1 && mv $p.out $p.ok
        if [ -f $p.ok ]; then
           Info Extract $p succeeded
           t=appimagetool; s=mksquashfs; z=zsyncmake; a=AppRun; r=runtime
           [[ ! -f bin/$t && -f AitDir/squashfs-root/usr/bin/$t ]] && \
           cp -f AitDir/squashfs-root/usr/bin/$t bin/ || :
           [[ ! -f bin/$s && -f AitDir/squashfs-root/usr/bin/$s ]] && \
           cp -f AitDir/squashfs-root/usr/bin/$s bin/ || :
           [[ ! -f bin/$z && -f AitDir/squashfs-root/usr/bin/$z ]] && \
           cp -f AitDir/squashfs-root/usr/bin/$z bin/ || :
           [[ ! -f bin/$a && -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$a ]] && \
           cp -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$a bin/ || :
           [[ ! -f bin/$r && -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$r ]] && \
           cp -f ${LP3D_DIST_DIR_PATH}/AppDir/tools/$r bin/ || :
        else
          Error Extract $p FAILED
          tail -80 $p.out
          exit 5
        fi
      Info Setup $p completed
    else
      Error Setup $p FAILED - file was not found.
      exit 5
    fi
    SaveAppImageSetupProgress
  else
    Info $p exists. Nothing to do.
  fi

  # lcConvert
  p=lconvert
  if [ ! -L ${AppDirBuildPath}/$p ]; then
    lcv="$(which $p)"
    Info "Linking to $p to $lcv..."
    if [ -n "$lcv" ]; then
      ( cd ${AppDirBuildPath} && ln -sf $lcv $p \
      ) >$p.out 2>&1 && rm -f $p.out
      if [ ! -f $p.out ]; then
        Info  "${AppDirBuildPath}/$p linked to $lcv" || :
        Info Link $p completed
      else
        Error Link $p FAILED
        tail -80 $p.out
        exit 5
      fi
      SaveAppImageSetupProgress
    else
      Error $p was not found
      exit 5
    fi
  else
    Info $p exists. Nothing to do.
  fi
fi

# Build AppImage
cd "${AppDirBuildPath}" || exit 1
Info && Info "Building AppImage from AppDirBuildPath: ${AppDirBuildPath}..."
renderers=$(find ./opt -type f);
for r in $renderers; do executables="$executables -executable=$r" && Info "Set executable $executables"; done;
unset QTDIR; unset QT_PLUGIN_PATH # no longer needed, superceded by AppRun
export VERSION="$LP3D_VERSION"    # used to construct the file name
./linuxdeployqt ./usr/share/applications/*.desktop $executables -bundle-non-qt-libs -verbose=2
if [[ -z "${LP3D_AI_BUILD_TOOLS}" && ("${LP3D_ARCH}" = "amd64" || "${LP3D_ARCH}" = "x86_64") ]]; then
  ./linuxdeployqt ./usr/share/applications/*.desktop -appimage -verbose=2
  AppImage=$(ls LPub3D*.AppImage)  # name with full path
elif [[ -n "${LP3D_AI_BUILD_TOOLS}" || "${LP3D_ARCH}" = "arm64" || "${LP3D_ARCH}" = "aarch64" || "${LP3D_QEMU}" = "true" ]]; then
  # lpub3d.desktop
  [ -f "./usr/share/applications/lpub3d.desktop" ] && \
  cp -f ./usr/share/applications/lpub3d.desktop . || \
  Error "./usr/share/applications/lpub3d.desktop was not found"

  # lpub3d.png and .DirIcon
  [ -f "./usr/share/icons/hicolor/128x128/apps/lpub3d.png" ] && \
  cp -f ./usr/share/icons/hicolor/128x128/apps/lpub3d.png . && \
  ln -sf ./usr/share/icons/hicolor/128x128/apps/lpub3d.png .DirIcon || \
  Error "./usr/share/icons/hicolor/128x128/apps/lpub3d.png was not found"

  # libffi
  if [ ! -f "./usr/lib/libffi.so.6" ]; then
    [ ! -d "./usr/lib/" ] && mkdir -p ./usr/lib || :
    # libffi is a runtime dynamic dependency
    # see this thread for more information on the topic:
    # https://mail.gnome.org/archives/gtk-devel-list/2012-July/msg00062.html
    if [[ "${LP3D_ARCH}" == "amd64" || "${LP3D_ARCH}" == "x86_64" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | grep x86-64 | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    elif [[ "${LP3D_ARCH}" == "amd32" || "${LP3D_ARCH}" == "i686" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | head -n1 | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    elif [[ "${LP3D_ARCH}" == "arm32" || "${LP3D_ARCH}" == "armhf" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | grep arm | grep hf | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    elif [[ "${LP3D_ARCH}" == "arm64" || "${LP3D_ARCH}" == "aarch64" ]]; then
      cp "$(ldconfig -p | grep libffi.so.6 | grep aarch64 | cut -d'>' -f2 | tr -d ' ')" ./usr/lib/
    else
      Error "WARNING: unknown architecture, not bundling libffi"
    fi
  fi

  cd "$WD" || exit 1

  # AppRun
  a=AppRun
  if [ -f "${AppDirBuildPath}/$a" ]; then
    Info "Using personalized AppRun script"
  elif [ -f "bin/$a" ]; then
   ( cp -f bin/$a ${AppDirBuildPath}/usr/bin/ && \
     cd ${AppDirBuildPath} && ln -sf ./usr/bin/$a $a \
   ) >$a.out 2>&1 && rm -f $a.out
   if [ -f $a.out ]; then
     Error Copy and link $p FAILED
     tail -80 $a.out
   fi
  else
    Error "File ./bin/$a was not found"
  fi

  # zsyncmake - see note above
  a=zsyncmake
  if [ -f "bin/$a" ]; then
   ( cp -f bin/$a ${AppDirBuildPath}/usr/bin/ \
   ) >$a.out 2>&1 && rm -f $a.out
   if [ -f $a.out ]; then
     Error Copy $p FAILED
     tail -80 $a.out
   fi
  else
   Error "File ./bin/$a was not found"
  fi

  # make AppImage
  AppImage=LPub3D-${LP3D_VERSION}-$(uname -m).AppImage  # name without path
  if [ -f "bin/mksquashfs" ]; then
    p=${AppImage}
    ( chmod a+x bin/mksquashfs && \
     ./bin/mksquashfs AppDir squashfs-root -root-owned -noappend \
    ) >$p.out 2>&1 && rm -f $p.out
    if [ ! -f $p.out ]; then
      AppImage=${WD}/${AppImage} # name with full path
      cat bin/runtime >> $p
      cat squashfs-root >> $p
    else
      Error Run Squashfs for $p FAILED
      tail -80 $p.out
      exit 5
    fi
  else
    Error "./bin/mksquashfs was not found"
    exit 7
  fi
fi

#Info && Info "AppImage Dynamic Library Dependencies:" && \
#find ./ -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq && Info

Info && Info "Confirm AppImage..."
if [ -f "${AppImage}" ]; then
  CommandArg=--appimage-version
  chmod a+x ${AppImage}
  if [[ ("${LP3D_ARCH}" = "arm64" || "${LP3D_ARCH}" = "aarch64") && -z "${LP3D_AI_MAGIC_BYTES}" ]]; then
    Info "Patch out AppImage magic bytes"
    p=AppImagePatch
    AppImageMagicBytes="$(hexdump -Cv ${AppImage} | head -n 1 | grep '41 49 02 00')"
    if [ -n "${AppImageMagicBytes}" ]; then
      formaterror="Exec format error"
      ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
      if [ "$(grep -F "${formaterror}" $p.out 2>/dev/null)" ]; then
        Info "AppImage magic bytes: ${AppImageMagicBytes}"
        Info "Patching out AppImage magic bytes..."
        cp -f ${AppImage} ${AppImage}.release
        dd if=/dev/zero of="${AppImage}" bs=1 count=3 seek=8 conv=notrunc
        if [ -z "$(hexdump -Cv ${AppImage} | head -n 1 | grep '41 49 02 00')" ]; then
          Info "Magic bytes patched: $(hexdump -Cv ${AppImage} | head -n 1)"
          ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
          if [ -f $p.ok ]; then
            cat $p.ok && rm $p.ok
            AppImagePatched=LPub3D-${LP3D_VERSION}-$(uname -m).AppImage.patched
            mv -f ${AppImage} ${AppImagePatched}
            Info "${AppImage} is runnable"
          else
            Error "Command ${AppImage} ${CommandArg} after magic bytes patch FAILED"
            tail -80 $p.out
            exit 7
          fi
        else
          Error "Patch AppImage magic bytes FAILED"
          hexdump -Cv ${AppImage} | head -n 3
          exit 7
        fi
      elif [ -f $p.ok ]; then
        cat $p.ok && rm $p.ok
        Info "${AppImage} is runnable"
      fi
    else
      Info "Magic bytes 'AI' not found in AppImage"
      hexdump -Cv ${AppImage} | head -n 3
      ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
      if [ -f $p.ok ]; then
        cat $p.ok && rm $p.ok
        Info "${AppImage} is runnable"
      else
        Error "Command ${AppImage} ${CommandArg} FAILED"
        tail -80 $p.out
        exit 7
      fi
    fi
  elif [ -n "${LP3D_AI_BUILD_TOOLS}" ]; then
    #CommandArg=--appimage-version
    ( ${AppImage} ${CommandArg} ) >$p.out 2>&1 && mv $p.out $p.ok
    if [ -f $p.ok ]; then
      cat $p.ok && rm $p.ok
      Info "${AppImage} is runnable"
    else
      Error "Command ${AppImage} ${CommandArg} FAILED"
      tail -80 $p.out
      exit 7
    fi
  fi

  # Rename AppImage and move to $PWD
  AppImageExtension=${AppImage##*-}
  AppImageName=LPub3D-${LP3D_APP_VERSION_LONG}-${AppImageExtension}
  [ -f ${AppImage}.release ] && \
  mv -f ${AppImage}.release ${AppImageName} || \
  mv -f ${AppImage} ${AppImageName}

  # Crete AppImage sha512 file
  AppImage=${AppImageName}
  Info "Creating hash file for ${AppImage}..."
  sha512sum "${AppImage}" > "${AppImage}.sha512" || \
  Error "Failed to create hash file ${AppImage}.sha512"
  Info "Application package....: ${AppImage}"
  Info "Package path...........: ${PWD}/${AppImage}"
  if [ -f "${AppImage}.sha512" ]; then
    Info "AppImage build completed successfully."
  else
    Error "AppImage build completed but the .sha512 file was not found."
  fi
else
  Error "${AppImage} build FAILED. File not found."
  exit 8
fi

cd "$WD" || exit 1

AppImage=$(find ./ -name ${AppImageName} -type f)

# Check AppImage build
[ -f "${AppImagePatched}" ] && \
AppImageCheck=$(find ./ -name ${AppImagePatched} -type f) || \
AppImageCheck=${AppImage}
if [ -f "${AppImageCheck}" ]; then
  export SOURCE_DIR=${WD}
  export LP3D_BUILD_OS="appimage"
  export LP3D_CHECK_STATUS="--version --app-paths"
  mkdir -p appImage_Check && cp -f ${AppImageCheck} appImage_Check/${AppImageName} && \
  Info "$(ls ./appImage_Check/*.AppImage) copied to check folder."
  if [[ -z "$(which fusermount)" || -n "${LP3D_AI_EXTRACT_PAYLOAD}" || "${LP3D_QEMU}" = "true" ]]; then
    ( cd appImage_Check && ./${AppImageName} --appimage-extract \
    ) >$p.out 2>&1 && rm -f $p.out
    if [ ! -f $p.out ]; then
      Info "Build check extracted AppImage payload..."
      LPUB3D_EXE="appImage_Check/squashfs-root/usr/bin/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
    else
      Error "Extract ${AppImageName} FAILED"
      tail -80 $p.out
    fi
  else
    Info "Build check AppImage..."
    LPUB3D_EXE="appImage_Check/${AppImageName}"
  fi
  if [ -n "${LPUB3D_EXE}" ]; then
    export LPUB3D_EXE
    set +x && source ${SOURCE_DIR}/builds/check/build_checks.sh && set -x
  fi
  if [[ "${AppImageCheck}" == *".patched"* ]]; then
    rm -fr ${AppImageCheck}
  fi
else
  Error "${AppImageCheck} not found, the build check cannot proceed."
  exit 8
fi

# Move AppImage build content to output
Info "Moving AppImage build assets and logs to output folder..."
[ ! "${BUILD_OPT}" = "verify" ] && \
mv -f ${AppImage}* /out/ 2>/dev/null || :
mv -f ${AppDirBuildPath}/*.log /out/ 2>/dev/null || :
mv -f ./*.log /out/ 2>/dev/null || :
mv -f ~/*.log /out/ 2>/dev/null || :
mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :

exit 0
