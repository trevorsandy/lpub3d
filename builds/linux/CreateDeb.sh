#!/bin/bash
# Trevor SANDY
# Last Update June 23, 2019
# Copyright (c) 2017 - 2019 by Trevor SANDY
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
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
export OBS=false # OpenSUSE Build Service flag must be set for CreateRenderers.sh - called by debian/rules

echo "Start $ME execution at $CWD..."

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d}"
echo "   LPUB3D SOURCE DIR......${LPUB3D}"

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# when running locally, use this block...
if [ "${TRAVIS}" != "true"  ]; then
    # Travis starting point: /home/travis/build/trevorsandy/lpub3d
    #
    # logging stuff - increment log file name
    f="${CWD}/$ME"
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
else
    # Travis starts in the clone directory (lpub3d/), so move outside
    cd ../
fi

echo "1. create DEB working directories in debbuild/..."
if [ ! -d debbuild/SOURCES ]
then
    mkdir -p debbuild/SOURCES
fi

cd debbuild/SOURCES

if [ "${TRAVIS}" != "true" ]; then
    echo "2. download source to SOURCES/..."
    git clone https://github.com/trevorsandy/${LPUB3D}.git
else
    echo "2. copy source to SOURCES/..."
    cp -rf "../../${LPUB3D}" .
fi

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "4. move ${LPUB3D}/ to ${LPUB3D}-${LP3D_APP_VERSION}/ in SOURCES/..."
SOURCE_DIR=${LPUB3D}-${LP3D_APP_VERSION}
mv -f ${LPUB3D} ${SOURCE_DIR}

echo "5. create cleaned tarball ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz from ${SOURCE_DIR}/"
tar -czf ../${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz ${SOURCE_DIR} \
        --exclude="${SOURCE_DIR}/builds/linux/standard" \
        --exclude="${SOURCE_DIR}/builds/macx" \
        --exclude="${SOURCE_DIR}/.travis.yml" \
        --exclude="${SOURCE_DIR}/.gitattributes" \
        --exclude="${SOURCE_DIR}/LPub3D.pro.user" \
        --exclude="${SOURCE_DIR}/README.md" \
        --exclude="${SOURCE_DIR}/_config.yml" \
        --exclude="${SOURCE_DIR}/.gitignore" \
        --exclude="${SOURCE_DIR}/appveyor.yml"

echo "6. download LDraw archive libraries to SOURCES/..."
# we pull in the library archives here because the lpub3d.spec file copies them
# to the extras location. This config thus supports both Suse OBS and Travis CI build procs.
if [ ! -f lpub3dldrawunf.zip ]
then
    curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o lpub3dldrawunf.zip
fi
if [ ! -f complete.zip ]
then
    curl -O $curlopts http://www.ldraw.org/library/updates/complete.zip
fi
if [ ! -f tenteparts.zip ]
then
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip
fi
if [ ! -f vexiqparts.zip ]
then
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip
fi

echo "7. extract ${SOURCE_DIR}/ to debbuild/..."
cd ../
if [  -d ${LPUB3D}-${LP3D_APP_VERSION} ]
then
   rm -rf ${LPUB3D}-${LP3D_APP_VERSION}
fi
tar zxf ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz

echo "8. copy debian/ configuration directory to ${SOURCE_DIR}/..."
cp -rf ${SOURCE_DIR}/builds/linux/obs/debian ${SOURCE_DIR}

echo "9. install ${LPUB3D} build dependencies [requires elevated access - sudo]..."
cd "${SOURCE_DIR}"
controlDeps=`grep Build-Depends debian/control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,`
sudo apt-get update -qq
sudo apt-get install -y $controlDeps

echo "10. build application package from ${SOURCE_DIR}/..."
chmod 755 debian/rules
/usr/bin/dpkg-buildpackage -us -uc

cd ../
DISTRO_FILE=`ls ${LPUB3D}_${LP3D_APP_VERSION}*.deb`
if [ -f ${DISTRO_FILE} ]
then
    echo "11-1. check deb package..."
    lintian ${DISTRO_FILE} ${SOURCE_DIR}/${LPUB3D}.dsc

    echo "11-2. Build-check ${DISTRO_FILE}"
    if [ ! -f "/usr/bin/update-desktop-database" ]; then
        echo "      Program update-desktop-database not found. Installing..."
        sudo apt-get install -y desktop-file-utils
    fi
    # Install package - here we use the distro file name
    echo "      11-2. Build-check install ${LPUB3D}..."
    sudo dpkg -i ${DISTRO_FILE}
    # Check if exe exist - here we use the executable name
    LPUB3D_EXE=lpub3d${LP3D_APP_VER_SUFFIX}
    if [ -f "/usr/bin/${LPUB3D_EXE}" ]; then
        # Check commands
        source ${SOURCE_DIR}/builds/check/build_checks.sh
        # Cleanup - here we use the package name
        echo "      11-2. Build-check uninstall ${LPUB3D}..."
        sudo dpkg -r ${LPUB3D}
    else
        echo "11-2. Build-check failed - /usr/bin/${LPUB3D_EXE} not found."
    fi

    echo "11-3. create LPub3D update and download packages..."
    IFS=_ read DEB_NAME DEB_VERSION DEB_EXTENSION <<< ${DISTRO_FILE}
    LP3D_DEB_APP_VERSION_LONG=${LP3D_APP_VERSION_LONG}
    LP3D_DEB_VERSION=${LP3D_VERSION}
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
        *)
            LP3D_PLATFORM_NAME="debian" ;;
        esac
        ;;
    esac;
    [ -n "$LP3D_PLATFORM_NAME" ] && \
    LP3D_DEB_APP_VERSION_LONG="${LP3D_DEB_APP_VERSION_LONG}-${LP3D_PLATFORM_NAME}" && \
    LP3D_DEB_VERSION="${LP3D_DEB_VERSION}-${LP3D_PLATFORM_NAME}" || true

    mv -f ${DISTRO_FILE} "LPub3D-${LP3D_DEB_APP_VERSION_LONG}-${DEB_EXTENSION}"
    if [ -f "LPub3D-${LP3D_DEB_APP_VERSION_LONG}-${DEB_EXTENSION}" ]; then
        echo "    Download package..: LPub3D-${LP3D_DEB_APP_VERSION_LONG}-${DEB_EXTENSION}"
    else
        echo "    ERROR - file not copied: LPub3D-${LP3D_DEB_APP_VERSION_LONG}-${DEB_EXTENSION}"
    fi

else
    echo "11. package ${DISTRO_FILE} not found"
fi

# Elapsed execution time
FinishElapsedTime
