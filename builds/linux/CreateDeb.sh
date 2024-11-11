#!/bin/bash
# Trevor SANDY
# Last Update November 11, 2024
# Copyright (C) 2017 - 2024 by Trevor SANDY
# Build LPub3D Linux deb distribution
# To run:
# $ chmod 755 CreateDeb.sh
# $ [options] && ./builds/linux/CreateDeb.sh
# [options]:
#  - LOCAL=false - local build - use local versus download renderer and library source
#  - DOCKER=true - using Docker image
#  - LPUB3D=lpub3d - repository name
#  - OBS=false - building locally
#  - LP3D_ARCH=amd64 - set build architecture
#  - PRESERVE=false - clone remote repository
#  - UPDATE_SH=false - update overwrite this script when building in local Docker
#  - DEB_EXTENSION=amd64.deb - distribution file suffix
#  - LOCAL_RESOURCE_PATH= - path (or Docker volume mount) where lpub3d and renderer sources and library archives are located
#  - XSERVER=false - use Docker host XMing/XSrv XServer
# NOTE: elevated access required for apt-get install, execute with sudo
# or enable user with no password sudo if running noninteractive - see
# docker-compose/dockerfiles for script example of sudo, no password user.

# LOCAL DOCKER RUN - set accordingly then cut and paste in console to run:
: <<'BLOCK_COMMENT'
UPDATE_SH="${UPDATE_SH:-true}"
if test "${UPDATE_SH}" = "true"; then \
cp -rf /user/resources/builds/linux/CreateDeb.sh . \
&& export LOCAL=true \
&& export DOCKER=true \
&& export LPUB3D=lpub3d \
&& export PRESERVE=true \
&& export LP3D_ARCH=amd64 \
&& export LOCAL_RESOURCE_PATH=/user/resources \
&& export XSERVER=false \
&& chmod a+x CreateDeb.sh \
&& ./CreateDeb.sh \
&& if test -d /buildpkg; then \
  sudo cp -f /user/rpmbuild/RPMS/`uname -m`/*.rpm /buildpkg/; \
  sudo cp -f /user/rpmbuild/BUILD/*.log /buildpkg/; \
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
DEB_EXTENSION=${DEB_EXTENSION:-$LP3D_ARCH.deb}
LOCAL_RESOURCE_PATH=${LOCAL_RESOURCE_PATH:-}

export OBS # OpenSUSE Build Service flag must be set for CreateRenderers.sh

# Format name and set WRITE_LOG - SOURCED if $1 is empty
ME="CreateDeb"
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

# when running on Travis-CI, use this block...
if [ "${TRAVIS}" = "true"  ]; then
    # Travis starting point: /home/travis/build/trevorsandy/lpub3d
    # Travis starts in the clone directory (lpub3d/), so move outside
    cd ../
fi

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

echo "1. create DEB working directories in debbuild/..."
if [ ! -d debbuild/SOURCES ]
then
    mkdir -p debbuild/SOURCES
fi

cd debbuild/
BUILD_DIR=$PWD
cd ${BUILD_DIR}/SOURCES

if [ "${TRAVIS}" != "true" ]; then
    if [ -d "/in" ]; then
        echo "2. copy input source to SOURCES/${LPUB3D}..."
        mkdir -p ${LPUB3D} && cp -rf /in/. ${LPUB3D}/
    else
        LPUB3D_REPO=$(find . -maxdepth 1 -type d -name "${LPUB3D}"-*)
        if [[ "${PRESERVE}" != "true" || ! -d "${LPUB3D_REPO}" ]]; then
            if [ "$LOCAL" = "true" ]; then
                echo "2. copy LOCAL ${LPUB3D} source to SOURCES/..."
                cp -rf ${LOCAL_RESOURCE_PATH}/${LPUB3D} ${LPUB3D}
                echo "2a.copy LOCAL ${LPUB3D} renderer source to SOURCES/..."
                cp -rf ${LOCAL_RESOURCE_PATH}/povray.tar.gz .
                cp -rf ${LOCAL_RESOURCE_PATH}/ldglite.tar.gz .
                cp -rf ${LOCAL_RESOURCE_PATH}/ldview.tar.gz .
            else
                echo "2. download ${LPUB3D} source to SOURCES/..."
                if [ -d "${LPUB3D_REPO}" ]; then
                    rm -rf ${LPUB3D_REPO}
                fi
                git clone https://github.com/trevorsandy/${LPUB3D}.git
            fi
        else
            echo "2. preserve ${LPUB3D} source in SOURCES/..."
            if [ -d "${LPUB3D_REPO}" ]; then
                mv -f ${LPUB3D_REPO} ${LPUB3D}
            fi
        fi
    fi
else
    echo "2. copy ${LPUB3D} source to SOURCES/..."
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

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

WORK_DIR=${LPUB3D}-${LP3D_APP_VERSION}
if [[ "${PRESERVE}" != "true" || ! -d ${WORK_DIR} ]]; then
    echo "4. move ${LPUB3D}/ to ${LPUB3D}-${LP3D_APP_VERSION}/ in SOURCES/..."
    if [ -d ${WORK_DIR} ]; then
        rm -rf ${WORK_DIR}
    fi
    mv -f ${LPUB3D} ${WORK_DIR}
else
    if [ "$LOCAL" = "true" ]; then
        echo "4. overwrite ${LPUB3D}-${LP3D_APP_VERSION}/ with ${LPUB3D}/ in SOURCES/..."
        cp -TRf ${LPUB3D}/ ${WORK_DIR}/
        rm -rf ${LPUB3D}
    else
        echo "4. preserve ${LPUB3D}-${LP3D_APP_VERSION}/ in SOURCES/..."
    fi
fi

echo "5. create cleaned tarball ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz from ${WORK_DIR}/"
tar -czf ../${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz \
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
        --exclude="builds/linux/CreateLinuxMulitArch.sh" \
        --exclude="builds/linux/CreatePkg.sh" \
        --exclude="builds/linux/CreateRpm.sh" \
        --exclude="builds/windows" \
        --exclude="builds/macx" \
        --exclude="lclib/tools" \
        ${WORK_DIR}

# we pull in the library archives here because the lpub3d.spec file copies them
# to the extras location. This config thus supports both Suse OBS and Travis CI build procs.
if [ "$LOCAL" = "true" ]; then
    echo "6. copy LOCAL LDraw archive libraries to SOURCES/..."
    [ ! -f lpub3dldrawunf.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/lpub3dldrawunf.zip .

    # Place a copy of the unofficial library at ./debbuild
    [ ! -f ../lpub3dldrawunf.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/lpub3dldrawunf.zip ../

    [ ! -f complete.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/complete.zip .

    [ ! -f tenteparts.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/tenteparts.zip .

    [ ! -f vexiqparts.zip ] && \
    cp -rf ${LOCAL_RESOURCE_PATH}/vexiqparts.zip .
else
    echo "6. download LDraw archive libraries to SOURCES/..."
    [ ! -f lpub3dldrawunf.zip ] && \
    curl $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/lpub3dldrawunf.zip -o lpub3dldrawunf.zip || :

    [ ! -f complete.zip ] && \
    curl -O $curlopts https://library.ldraw.org/library/updates/complete.zip || :

    [ ! -f tenteparts.zip ] && \
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip || :

    [ ! -f vexiqparts.zip ] && \
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip || :
fi

[ -d ../lpub3d_linux_3rdparty ] && \
(cd ../lpub3d_linux_3rdparty && ln -sf ../SOURCES/lpub3dldrawunf.zip lpub3dldrawunf.zip) || :

[ -d ../lpub3d_linux_3rdparty ] && \
(cd ../lpub3d_linux_3rdparty && ln -sf ../SOURCES/complete.zip complete.zip) || :

echo "7. extract ${WORK_DIR}/ to debbuild/..."
cd ${BUILD_DIR}/
if [  -d ${LPUB3D}-${LP3D_APP_VERSION} ]
then
   rm -rf ${LPUB3D}-${LP3D_APP_VERSION}
fi
tar zxf ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz

echo "8. copy debian/ configuration directory to ${WORK_DIR}/..."
cp -rf ${WORK_DIR}/builds/linux/obs/debian ${WORK_DIR}

cd "${BUILD_DIR}/${WORK_DIR}/"

if [ "${CI}" = "true" ]; then
    echo "9. Skipping install ${LPUB3D} build dependencies."
else
    echo "9. install ${LPUB3D} build dependencies [requires elevated access - sudo]..."
    controlDeps=`grep Build-Depends debian/control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,`
    sudo apt-get update -qq
    sudo apt-get install -y $controlDeps
fi

if [ "$GITHUB" = "true" ]; then
    export DEBUILD_PRESERVE_ENVVARS="CI,OBS,GITHUB,DOCKER,LP3D_*"
fi
echo "10-1. build application package from ${BUILD_DIR}/${WORK_DIR}/..."
chmod 755 debian/rules
/usr/bin/dpkg-buildpackage -us -uc || exit 1

if [ "$LOCAL" = "true" ]; then
    cd ../
else
    cd ${BUILD_DIR}/
fi

DISTRO_FILE=`ls ${LPUB3D}_${LP3D_APP_VERSION}*.deb`

echo "10-2. Build package: $PWD/${DISTRO_FILE}"
if [ -f ${DISTRO_FILE} ]
then
    echo "11-1. Check (lintian) deb package..."
    lintian ${DISTRO_FILE} ${WORK_DIR}/${LPUB3D}.dsc
    if [ -n "$LP3D_SKIP_BUILD_CHECK" ]; then
        echo "11-2. Skipping ${DISTRO_FILE} build check."
    else
        if [ "${LP3D_QEMU}" = "true" ]; then
            if [ -n "$LP3D_PRE_PACKAGE_CHECK" ]; then
                echo "11-2. Pre-package build check LPub3D..."
                export LP3D_BUILD_OS=
                export SOURCE_DIR=${BUILD_DIR}/${WORK_DIR}
                export LP3D_CHECK_LDD="1"
                export LP3D_CHECK_STATUS="--version --app-paths"
                case ${LP3D_ARCH} in
                    "aarch64"|"arm64")
                        LP3D_BUILD_ARCH="64bit_release" ;;
                    *)
                        LP3D_BUILD_ARCH="32bit_release" ;;
                esac
                export LPUB3D_EXE="${SOURCE_DIR}/mainApp/${LP3D_BUILD_ARCH}/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
                (cd ${SOURCE_DIR} && chmod a+x builds/check/build_checks.sh && ./builds/check/build_checks.sh)
            else
                echo "11-2. Building in QEMU, skipping build check."
            fi
        else
            echo "11-2. Build check ${DISTRO_FILE}"
            if [ ! -f "/usr/bin/update-desktop-database" ]; then
                echo "      Program update-desktop-database not found. Installing..."
                sudo apt-get install -y desktop-file-utils
            fi
            # Install package - here we use the distro file name
            echo "      11-2. Build check install ${LPUB3D}..."
            sudo dpkg -i ${DISTRO_FILE}
            # Check if exe exist - here we use the executable name
            LPUB3D_EXE=lpub3d${LP3D_APP_VER_SUFFIX}
            SOURCE_DIR=${WORK_DIR}
            if [ -f "/usr/bin/${LPUB3D_EXE}" ]; then
                # Check commands
                LP3D_CHECK_LDD="1"
                source ${WORK_DIR}/builds/check/build_checks.sh
                # Cleanup - here we use the package name
                echo "      11-2. Build check uninstall ${LPUB3D}..."
                sudo dpkg -r ${LPUB3D}
            else
                echo "11-2. Build check failed - /usr/bin/${LPUB3D_EXE} not found."
            fi
        fi
    fi

    # Stop here if build option is verification only
    if [ "$BUILD_OPT" = "verify" ]; then
        echo "11-3. Cleanup build assets..."
        rm -f ./*.deb* 2>/dev/null || :
        rm -f ./*.xz 2>/dev/null || :
        if [ "${LP3D_QEMU}" = "true" ]; then
            echo "11-4. Moving ${LP3D_BASE} ${LP3D_ARCH} logs to output folder..."
            mv -f ${SOURCE_DIR}/*.log /out/ 2>/dev/null || :
            mv -f ${CWD}/*.log /out/ 2>/dev/null || :
            mv -f ./*.log /out/ 2>/dev/null || :
            mv -f ~/*.log /out/ 2>/dev/null || :
            mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :
        fi
        exit 0
    fi

    IFS=_ read DEB_NAME DEB_VERSION DEB_EXTENSION <<< ${DISTRO_FILE}
    echo "11-3. Create LPub3D ${DEB_EXTENSION} distribution packages..."
    LP3D_PLATFORM_ID=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $(uname) | awk '{print tolower($0)}')
    LP3D_PLATFORM_VER=$(. /etc/os-release 2>/dev/null; [ -n "$VERSION_ID" ] && echo $VERSION_ID || true)
    case ${LP3D_PLATFORM_ID} in
    ubuntu)
        case ${LP3D_PLATFORM_VER} in
        14.04)
            LP3D_PLATFORM_NAME="trusty" ;;
        16.04)
            LP3D_PLATFORM_NAME="xenial" ;;
        16.10)
            LP3D_PLATFORM_NAME="yakkety" ;;
        17.04)
            LP3D_PLATFORM_NAME="zesty" ;;
        17.10)
            LP3D_PLATFORM_NAME="artful" ;;
        18.04)
            LP3D_PLATFORM_NAME="bionic" ;;
        18.10)
            LP3D_PLATFORM_NAME="cosmic" ;;
        19.04)
            LP3D_PLATFORM_NAME="disco" ;;
        19.10)
            LP3D_PLATFORM_NAME="eoan" ;;
        20.04)
            LP3D_PLATFORM_NAME="focal" ;;
        20.10)
            LP3D_PLATFORM_NAME="groovy" ;;
        21.04)
            LP3D_PLATFORM_NAME="hirsute" ;;
        21.10)
            LP3D_PLATFORM_NAME="impish" ;;
        22.04)
            LP3D_PLATFORM_NAME="jammy" ;;
        22.10)
            LP3D_PLATFORM_NAME="kinetic" ;;
        23.04)
            LP3D_PLATFORM_NAME="lunar" ;;
        23.10)
            LP3D_PLATFORM_NAME="mantic" ;;
        24.04)
            LP3D_PLATFORM_NAME="noble" ;;
        24.10)
            LP3D_PLATFORM_NAME="oracular" ;;
        *)
            LP3D_PLATFORM_NAME="ubuntu" ;;
        esac
        ;;
    debian)
        case ${LP3D_PLATFORM_VER} in
        8)
            LP3D_PLATFORM_NAME="jessie" ;;
        9)
            LP3D_PLATFORM_NAME="stretch" ;;
        10)
            LP3D_PLATFORM_NAME="buster" ;;
        11)
            LP3D_PLATFORM_NAME="bullseye" ;;
        12)
            LP3D_PLATFORM_NAME="bookworm" ;;
        *)
            LP3D_PLATFORM_NAME="debian" ;;
        esac
        ;;
    esac;
    [ -n "$LP3D_PLATFORM_NAME" ] && \
    LP3D_DEB_APP_VERSION_LONG="${LP3D_APP_VERSION_LONG}-${LP3D_PLATFORM_NAME}" || \
    LP3D_DEB_APP_VERSION_LONG="${LP3D_APP_VERSION_LONG}-ubuntu"
    LP3D_DEB_FILE="LPub3D-${LP3D_DEB_APP_VERSION_LONG}-${DEB_EXTENSION}"
    mv -f ${DISTRO_FILE} "${LP3D_DEB_FILE}"
    if [ -f "${LP3D_DEB_FILE}" ]; then
        if [ "${TRAVIS}" != "true" ]; then
            echo "11-4. Creating ${LP3D_DEB_FILE}.sha512 hash file..."
            sha512sum "${LP3D_DEB_FILE}" > "${LP3D_DEB_FILE}.sha512" || \
            echo "11-4. ERROR - Failed to create hash file ${LP3D_DEB_FILE}.sha512"
        fi
        if [ "${LP3D_QEMU}" = "true" ]; then
            echo "11-5. Moving ${LP3D_BASE} ${LP3D_ARCH} build assets and logs to output folder..."
            mv -f ${BUILD_DIR}/*.deb* /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.xz /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.dsc /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.changes /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.buildinfo /out/ 2>/dev/null || :
            mv -f ${SOURCE_DIR}/*.log /out/ 2>/dev/null || :
            mv -f ${CWD}/*.log /out/ 2>/dev/null || :
            mv -f ./*.log /out/ 2>/dev/null || :
            mv -f ~/*.log /out/ 2>/dev/null || :
            mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :

        fi
        echo
        echo "    Distribution package.: ${LP3D_DEB_FILE}"
        echo "    Package path.........: $PWD/${LP3D_DEB_FILE}"
    else
        echo "11-3. ERROR - file  ${LP3D_DEB_FILE} not found"
    fi
else
    echo "10-2. ERROR - package ${DISTRO_FILE} not found"
fi

exit 0
