#!/bin/bash
# Trevor SANDY
# Last Update March 14, 2025
# Copyright (C) 2017 - 2025 by Trevor SANDY
# Build LPub3D Linux rpm distribution
# To run:
# $ chmod 755 CreatePkg.sh
# $ [options] && ./builds/linux/CreatePkg.sh
# [options]:
#  - LOCAL=false - local build - use local versus download renderer and library source
#  - DOCKER=true - using Docker image
#  - LPUB3D=lpub3d - repository name
#  - OBS=false - building locally
#  - LP3D_ARCH=amd64 - set build architecture
#  - PRESERVE=false - clone remote repository
#  - UPDATE_SH=false - update overwrite this script when building in local Docker
#  - LOCAL_RESOURCE_PATH= - path (or Docker volume mount) where lpub3d and renderer sources and library archives are located
#  - XSERVER=false - use Docker host XMing/XSrv XServer
# NOTE: elevated access required for apt-get install, execute with sudo
# or enable user with no password sudo if running noninteractive - see
# docker-compose/dockerfiles for script example of sudo, no password user.

# LOCAL DOCKER RUN - set accordingly then cut and paste in console to run:
: <<'BLOCK_COMMENT'
UPDATE_SH="${UPDATE_SH:-true}"
if test "${UPDATE_SH}" = "true"; then \
cp -rf /user/resources/builds/linux/CreatePkg.sh . \
&& export LOCAL=true \
&& export DOCKER=true \
&& export LPUB3D=lpub3d \
&& export PRESERVE=true \
&& export LP3D_ARCH=amd64 \
&& export LOCAL_RESOURCE_PATH=/user/resources \
&& export XSERVER=false \
&& chmod a+x CreatePkg.sh \
&& ./CreatePkg.sh \
&& if test -d /buildpkg; then \
  sudo cp -f /user/pkgbuild/*.pkg.tar.xz /buildpkg/; \
  sudo cp -f /user/pkgbuild/src/*.log /buildpkg/; \
  sudo cp -f /user/*.log /buildpkg/; \
fi

BLOCK_COMMENT

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  [ -n "${LP3D_ARCH}" ] && ME="${ME} for (${LP3D_ARCH})" || ME="${ME} for (amd64)"
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "$ME Verification Finished!"
  else
    echo "$ME Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

CWD=`pwd`

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
OBS=${OBS:-false}
LOCAL=${LOCAL:-}
LPUB3D=${LPUB3D:-lpub3d}
DOCKER=${DOCKER:-}
XSERVER=${XSERVER:-}
PRESERVE=${PRESERVE:-} # preserve cloned repository
LP3D_ARCH=${LP3D_ARCH:-amd64}
LOCAL_RESOURCE_PATH=${LOCAL_RESOURCE_PATH:-}

export OBS # OpenSUSE Build Service flag must be set for CreateRenderers.sh - called by PKGBUILD

# Format name and set WRITE_LOG - SOURCED if $1 is empty
ME="CreatePkg"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

# automatic logging
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
if [ "${WRITE_LOG}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"; [ -n "${LP3D_ARCH}" ] && f="${f}-${LP3D_ARCH}" || f="${f}-amd64"
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

export LP3D_LOG_PATH

if [ "${XSERVER}" = "true" ]; then
        if test "${LOCAL}" != "true"; then export XSERVER=; fi
fi

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

echo "Start $ME execution at $CWD..."

echo "   LPUB3D SOURCE DIR......${LPUB3D}"
echo "   LPUB3D BUILD ARCH......${LP3D_ARCH}"
echo "   LPUB3D BUILD PLATFORM..$([ "$DOCKER" = "true" ] && echo "DOCKER" || echo "HOST RUNNER")"
if [ "$LOCAL" = "true" ]; then
    echo "   LPUB3D BUILD TYPE......Local"
    echo "   LPUB3D BUILD DISPLAY...$(if test "${XSERVER}" = "true"; then echo XSERVER; else echo XVFB; fi)"
    echo "   UPDATE BUILD SCRIPT....$(if test "${UPDATE_SH}" = "true"; then echo YES; else echo NO; fi)"
    echo "   PRESERVE BUILD REPO....$(if test "${PRESERVE}" = "true"; then echo YES; else echo NO; fi)"
    if [ -n "$LOCAL_RESOURCE_PATH" ]; then
        echo "   LOCAL_RESOURCE_PATH....${LOCAL_RESOURCE_PATH}"
    else
        echo "ERROR - LOCAL_RESOURCE_PATH was not specified. $ME will terminate."
        exit 1
    fi
else
    echo "   LPUB3D BUILD TYPE......CI"
fi

echo "1. create PKG working directories in pkgbuild/"
if [ ! -d pkgbuild/upstream ]
then
  mkdir -p pkgbuild/upstream
fi

cd pkgbuild/
BUILD_DIR=$PWD
cd ${BUILD_DIR}/upstream

if [ "${TRAVIS}" != "true" ]; then
    if [ -d "/in" ]; then
        echo "2. copy input source to upstream/${LPUB3D}..."
        mkdir -p ${LPUB3D} && cp -rf /in/. ${LPUB3D}/
    else
        LPUB3D_REPO=$(find . -maxdepth 1 -type d -name "${LPUB3D}"-*)
        if [[ "${PRESERVE}" != "true" || ! -d "${LPUB3D_REPO}" ]]; then
            if [ "$LOCAL" = "true" ]; then
                echo "2. copy LOCAL ${LPUB3D} source to upstream/..."
                cp -rf ${LOCAL_RESOURCE_PATH}/${LPUB3D} ${LPUB3D}
            else
                echo "2. download ${LPUB3D} source to upstream/..."
                if [ -d "${LPUB3D_REPO}" ]; then
                    rm -rf ${LPUB3D_REPO}
                fi
                git clone https://github.com/trevorsandy/${LPUB3D}.git
            fi
        else
            echo "2. preserve ${LPUB3D} source in upstream/..."
            if [ -d "${LPUB3D_REPO}" ]; then
                mv -f ${LPUB3D_REPO} ${LPUB3D}
            fi
        fi
    fi
else
    echo "2. copy ${LPUB3D} source to upstream/..."
    cp -rf "../../${LPUB3D}" .
fi

# For Docker build, check if there is a tag after the last commit
if [ "$DOCKER" = "true" ]; then
   # Setup git command
   GIT_CMD="git --git-dir $PWD/${LPUB3D}/.git --work-tree $PWD/${LPUB3D}"
   # Update source
   [ "${CI}" = "true" ] && $GIT_CMD pull || :
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

_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

WORK_DIR=${LPUB3D}-git
if [[ "${PRESERVE}" != "true" || ! -d ${WORK_DIR} ]]; then
    echo "3. move ${LPUB3D}/ to ${LPUB3D}-git/ in upstream/..."
    if [ -d ${WORK_DIR} ]; then
        rm -rf ${WORK_DIR}
    fi
    mv -f ${LPUB3D} ${WORK_DIR}
else
    if [ "$LOCAL" = "true" ]; then
        echo "3. overwrite ${LPUB3D}-git/ with ${LPUB3D}/ in upstream/..."
        cp -TRf ${LPUB3D}/ ${WORK_DIR}/
        rm -rf ${LPUB3D}
    else
        echo "3. preserve ${LPUB3D}-git/ in upstream/..."
    fi
fi

echo "4. create tarball ${WORK_DIR}.tar.gz from ${WORK_DIR}/"
tar -czf ../${WORK_DIR}.tar.gz \
        --exclude=".gitignore" \
        --exclude=".gitattributes" \
        --exclude=".travis.yml" \
        --exclude="LPub3D.pro.user" \
        --exclude="appveyor.yml" \
        --exclude="README.md" \
        --exclude="builds/utilities/Copyright-Source-Header.txt" \
        --exclude="builds/utilities/create-dmg" \
        --exclude="builds/utilities/CreateRenderers.bat" \
        --exclude="builds/utilities/README.md" \
        --exclude="builds/utilities/set-ldrawdir.command" \
        --exclude="builds/utilities/update-config-files.bat" \
        --exclude="builds/utilities/cert" \
        --exclude="builds/utilities/ci" \
        --exclude="builds/utilities/dmg-utils" \
        --exclude="builds/utilities/hooks" \
        --exclude="builds/utilities/icons" \
        --exclude="builds/utilities/json" \
        --exclude="builds/utilities/nsis-scripts" \
        --exclude="builds/linux/docker-compose" \
        --exclude="builds/linux/standard" \
        --exclude="builds/linux/CreateDeb.sh" \
        --exclude="builds/linux/CreateLinuxMulitArch.sh" \
        --exclude="builds/linux/CreateRpm.sh" \
        --exclude="builds/windows" \
        --exclude="builds/macx" \
        --exclude="lclib/tools" \
        ${WORK_DIR}

echo "5. copy PKGBUILD to pkgbuild/"
cp -f "${WORK_DIR}/builds/linux/obs/PKGBUILD" ${BUILD_DIR}
# pkgbuild
cd ${BUILD_DIR}

echo "5a. add LP3D_LOG_PATH to PKGBUILD"
[ -f "PKGBUILD" ] && \
sed -i -e "s;^	export LP3D_LOG_PATH=;	export LP3D_LOG_PATH=\"${LP3D_LOG_PATH}\";" \
       -e "s;^	export LP3D_CPU_CORES=;	export LP3D_CPU_CORES=\"${LP3D_CPU_CORES}\";" \
       -e "s;^	export LP3D_3RD_DIST_DIR=;	export LP3D_3RD_DIST_DIR=\"${LP3D_3RD_DIST_DIR}\";" \
       "PKGBUILD" || :

if [ "$LOCAL" = "true" ]; then
    echo "6. copy LOCAL LDraw archive libraries to pkgbuild/..."
    [ ! -f lpub3dldrawunf.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/lpub3dldrawunf.zip .

    [ ! -f complete.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/complete.zip .

    [ ! -f tenteparts.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/tenteparts.zip .

    [ ! -f vexiqparts.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/vexiqparts.zip .
else
    echo "6. download LDraw archive libraries to pkgbuild/..."
    [ ! -f lpub3dldrawunf.zip ] && \
    curl $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/lpub3dldrawunf.zip -o lpub3dldrawunf.zip || :

    [ ! -f complete.zip ] && \
    curl -O $curlopts https://library.ldraw.org/library/updates/complete.zip || :

    [ ! -f tenteparts.zip ] && \
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip || :

    [ ! -f vexiqparts.zip ] && \
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip || :
fi

# download 3rd party packages defined as source in pkgbuild/
if [ "$LOCAL" = "true" ]; then
    echo "7. LOCAL ${LPUB3D} renderer source to pkgbuild/..."
    cp -rf ${LOCAL_RESOURCE_PATH}/povray.tar.gz .
    cp -rf ${LOCAL_RESOURCE_PATH}/ldglite.tar.gz .
    cp -rf ${LOCAL_RESOURCE_PATH}/ldview.tar.gz .
else
    echo "7. download ${LPUB3D} renderer source to pkgbuild/..."
fi
for buildDir in ldglite ldview povray; do
  case ${buildDir} in
  ldglite)
    curlCommand="https://github.com/trevorsandy/ldglite/archive/master.tar.gz"
    ;;
  ldview)
    curlCommand="https://github.com/trevorsandy/ldview/archive/lpub3d-build.tar.gz"
    ;;
  povray)
    curlCommand="https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.tar.gz"
    ;;
  esac
  if [ ! -f ${buildDir}.tar.gz ]; then
    Info "`echo ${buildDir} | awk '{print toupper($0)}'` tarball ${buildDir}.tar.gz does not exist. Downloading..."
    curl $curlopts -o ${buildDir}.tar.gz ${curlCommand}
  fi
done

echo "8-1. build application package"
if [ "${LP3D_QEMU}" != "true" ]; then
    makepkg --syncdeps --noconfirm --needed || exit 1
else
    makepkg --noconfirm || exit 1
fi

DISTRO_FILE=`ls ${LPUB3D}-${LP3D_APP_VERSION}*.pkg.tar.zst`
if [ -f "${DISTRO_FILE}" ]
then
    echo "8-2. Build package: $PWD/${DISTRO_FILE}"
    if [ -n "$LP3D_SKIP_BUILD_CHECK" ]; then
        echo "9. Skipping ${DISTRO_FILE} build check."
    else
        if [ "${LP3D_QEMU}" = "true" ]; then
            if [ -n "$LP3D_PRE_PACKAGE_CHECK" ]; then
                echo "9-1. Pre-package build check LPub3D..."
                export LP3D_BUILD_OS=
                export SOURCE_DIR=${BUILD_DIR}/src/${WORK_DIR}
                export LP3D_CHECK_LDD="1"
                export LP3D_CHECK_STATUS="--version --app-paths"
                case ${LP3D_ARCH} in
                    "aarch64"|"arm64")
                        LP3D_BUILD_ARCH="64bit_release" ;;
                    *)
                        LP3D_BUILD_ARCH="32bit_release" ;;
                esac
                export LPUB3D_EXE="${BUILD_DIR}/src/${WORK_DIR}/mainApp/${LP3D_BUILD_ARCH}/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
                (cd ${SOURCE_DIR} && chmod a+x builds/check/build_checks.sh && ./builds/check/build_checks.sh)
            else
                echo "9-1. Building in QEMU, skipping build check."
            fi
	    
        else
            echo "9-1. Build check ${DISTRO_FILE}"
            if [ ! -f "/usr/bin/update-desktop-database" ]; then
                echo "      Program update-desktop-database not found. Installing..."
                sudo pacman -S --noconfirm --needed desktop-file-utils
            fi
            # Install package - here we use the distro file name e.g. LPub3D-2.3.8.1566-1-x86_64.pkg.tar.zst
            echo "      9-1. Build check install ${LPUB3D}..."
            sudo pacman -U --noconfirm ${DISTRO_FILE}
            # Check if exe exist - here we use the executable name e.g. lpub3d22
            LPUB3D_EXE=lpub3d${LP3D_APP_VER_SUFFIX}
            SOURCE_DIR=src/${WORK_DIR}
            if [ -f "/usr/bin/${LPUB3D_EXE}" ]; then
                # Check commands
                LP3D_CHECK_LDD="1"
                source ${SOURCE_DIR}/builds/check/build_checks.sh
                echo "      9-1. Build check uninstall ${LPUB3D}..."
                # Cleanup - here we use the package name e.g. lpub3d
                sudo pacman -Rs --noconfirm ${LPUB3D}
            else
                echo "9-1. Build check failed - /usr/bin/${LPUB3D_EXE} not found."
            fi
        fi
	fi

    # Stop here if build option is verification only
    if [ "$BUILD_OPT" = "verify" ]; then
        echo "9-2. Cleanup build assets..."
        rm -f ./*.pkg.tar.zst 2>/dev/null || :
        if [ "${LP3D_QEMU}" = "true" ]; then
            echo "9-3. Moving ${LP3D_BASE} ${LP3D_ARCH} logs to output folder..."
            mv -f ${BUILD_DIR}/*.log /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/src/*.log /out/ 2>/dev/null || :
            mv -f ${SOURCE_DIR}/*.log /out/ 2>/dev/null || :
            mv -f ${CWD}/*.log /out/ 2>/dev/null || :
            mv -f ~/*.log /out/ 2>/dev/null || :
            mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :
        fi
        exit 0
    fi

    IFS=- read PKG_NAME PKG_VERSION BUILD PKG_EXTENSION <<< ${DISTRO_FILE}
    echo "9-2. create LPub3D ${PKG_EXTENSION} distribution packages"
    LP3D_PKG_FILE="LPub3D-${LP3D_APP_VERSION_LONG}-${PKG_EXTENSION}"
    mv -f ${DISTRO_FILE} "${LP3D_PKG_FILE}"
    if [ -f "${LP3D_PKG_FILE}" ]; then
        if [ "${TRAVIS}" != "true" ]; then
            echo "9-3. Creating ${LP3D_PKG_FILE}.sha512 hash file..."
            sha512sum "${LP3D_PKG_FILE}" > "${LP3D_PKG_FILE}.sha512" || \
            echo "9-3. ERROR - Failed to create hash file ${LP3D_PKG_FILE}.sha512"
        fi
        if [ "${LP3D_QEMU}" = "true" ]; then
            echo "9-4. Moving ${LP3D_BASE} ${LP3D_ARCH} build assets and logs to output folder..."
            mv -f ${BUILD_DIR}/*.zst* /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.log /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/src/*.log /out/ 2>/dev/null || :
            mv -f ${SOURCE_DIR}/*.log /out/ 2>/dev/null || :
            mv -f ${CWD}/*.log /out/ 2>/dev/null || :
            mv -f ~/*.log /out/ 2>/dev/null || :
            mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :
        fi
        echo "    Distribution package.: ${LP3D_PKG_FILE}"
        echo "    Package path.........: $PWD/${LP3D_PKG_FILE}"
    else
        echo "9-2. ERROR - file not copied: ${LP3D_PKG_FILE}"
    fi
else
    echo "8-2. package ${DISTRO_FILE} not found."
fi

exit 0
