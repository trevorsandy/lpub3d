#!/bin/bash
# Trevor SANDY
# Last Update August 06, 2021
# Copyright (c) 2017 - 2021 by Trevor SANDY
# To run:
# $ chmod 755 CreateDeb.sh
# $ [options] && ./builds/linux/CreateDeb.sh
# [options]:
#  - export DOCKER=true (if using Docker image)
#  - export OBS=false (if building locally)
# NOTE: elevated access required for apt-get install, execute with sudo
# or enable user with no password sudo if running noninteractive - see
# docker-compose/dockerfiles for script example of sudo, no password user.

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  [ -n "${LP3D_ARCH}" ] && ME="${ME} for (${LP3D_ARCH})" || ME="${ME} for (amd64)"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# Format the log name - SOURCED if $1 is empty 
WRITE_LOG=${WRITE_LOG:-true}
[ "$1" = "" ] && WRITE_LOG="false" && ME="CreateDeb" || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

CWD=`pwd`
export OBS=false # OpenSUSE Build Service flag must be set for CreateRenderers.sh - called by debian/rules

echo "Start $ME execution at $CWD..."

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d}"
echo "   LPUB3D SOURCE DIR......${LPUB3D}"

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# logging stuff - increment log file name
if [ "${WRITE_LOG}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"; [ -n "${LP3D_ARCH}" ] && f="${f}-${LP3D_ARCH}" || f="${f}-amd64"
    [ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
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

# when running on Travis-CI, use this block...
if [ "${TRAVIS}" = "true"  ]; then
    # Travis starting point: /home/travis/build/trevorsandy/lpub3d
    # Travis starts in the clone directory (lpub3d/), so move outside
    cd ../
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
        echo "2. download ${LPUB3D} source to SOURCES/..."
        git clone https://github.com/trevorsandy/${LPUB3D}.git
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

echo "4. move ${LPUB3D}/ to ${LPUB3D}-${LP3D_APP_VERSION}/ in SOURCES/..."
WORK_DIR=${LPUB3D}-${LP3D_APP_VERSION}
mv -f ${LPUB3D} ${WORK_DIR}

echo "5. create cleaned tarball ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz from ${WORK_DIR}/"
tar -czf ../${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz ${WORK_DIR} \
        --exclude="${WORK_DIR}/builds/linux/standard" \
        --exclude="${WORK_DIR}/builds/macx" \
        --exclude="${WORK_DIR}/.travis.yml" \
        --exclude="${WORK_DIR}/.gitattributes" \
        --exclude="${WORK_DIR}/LPub3D.pro.user" \
        --exclude="${WORK_DIR}/README.md" \
        --exclude="${WORK_DIR}/_config.yml" \
        --exclude="${WORK_DIR}/.gitignore" \
        --exclude="${WORK_DIR}/snapcraft.yaml" \
        --exclude="${WORK_DIR}/appveyor.yml"

echo "6. download LDraw archive libraries to SOURCES/..."
# we pull in the library archives here because the lpub3d.spec file copies them
# to the extras location. This config thus supports both Suse OBS and Travis CI build procs.
[ ! -f lpub3dldrawunf.zip ] && \
curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o lpub3dldrawunf.zip || :
[ -d ../lpub3d_linux_3rdparty ] && \
(cd ../lpub3d_linux_3rdparty && ln -sf ../SOURCES/lpub3dldrawunf.zip lpub3dldrawunf.zip) || :
[ ! -f complete.zip ] && \
curl -O $curlopts http://www.ldraw.org/library/updates/complete.zip || :
[ -d ../lpub3d_linux_3rdparty ] && \
(cd ../lpub3d_linux_3rdparty && ln -sf ../SOURCES/complete.zip complete.zip) || :
[ ! -f tenteparts.zip ] && \
curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip || :
[ ! -f vexiqparts.zip ] && \
curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip || :

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
/usr/bin/dpkg-buildpackage -us -uc
cd ${BUILD_DIR}/
DISTRO_FILE=`ls ${LPUB3D}_${LP3D_APP_VERSION}*.deb`

echo "10-2. Build package: $PWD/${DISTRO_FILE}"
if [ -f ${DISTRO_FILE} ]
then
    echo "11-1. Check (lintian) deb package..."
    lintian ${DISTRO_FILE} ${WORK_DIR}/${LPUB3D}.dsc

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

    echo "11-3. create LPub3D ${DEB_EXTENSION} distribution packages..."
    IFS=_ read DEB_NAME DEB_VERSION DEB_EXTENSION <<< ${DISTRO_FILE}
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
            LP3D_PLATFORM_NAME="groovy" ;;
        20.10)
            LP3D_PLATFORM_NAME="focal" ;;
        21.04)
            LP3D_PLATFORM_NAME="Hirsute" ;;
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
