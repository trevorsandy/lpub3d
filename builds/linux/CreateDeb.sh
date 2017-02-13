#!/bin/bash
# Trevor SANDY
# Last Update 12 February 2017
# To run:
# $ chmod 755 CreateDeb.sh
# $ ./CreateDeb.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
BUILD_DATE=`date "+%Y%m%d"`

# logging stuff
WORK_DIR=`pwd`
LOG="${WORK_DIR}/$ME.log"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start $ME..."
echo "1. create DEB working directories"
if [ ! -d debbuild ]
then
    mkdir debbuild
    mkdir debbuild/upstream
else
   rm -rf debbuild
   mkdir debbuild
   mkdir debbuild/upstream
fi
cd debbuild/upstream

echo "2. download source"
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. capture version info"
#         1 2  3  4   5       6    7  8  9       10
# format "2 0 20 17 663 410fdd7 2017 02 12 19:50:21"
FILE="lpub3d/builds/utilities/version.info"
if [ -f ${FILE} -a -r ${FILE} ]
then
    VERSION_INFO=`cat ${FILE}`
else
    echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
    echo "$ME terminated!"
    exit 1
fi
read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION  VER_BUILD VER_SHA_HASH VER_REST <<< ${VERSION_INFO//'"'}
VERSION=${VER_MAJOR}"."${VER_MINOR}"."${VER_PATCH}
APP_VERSION=${VERSION}"."${VER_BUILD}
APP_VERSION_LONG=${VERSION}"."${VER_REVISION}"."${VER_BUILD}_${BUILD_DATE}
echo "WORK_DIR..........${WORK_DIR}"
echo "VER_MAJOR.........${VER_MAJOR}"
echo "VER_MINOR.........${VER_MINOR}"
echo "VER_PATCH.........${VER_PATCH}"
echo "VER_REVISION......${VER_REVISION}"
echo "VER_BUILD.........${VER_BUILD}"
echo "VER_SHA_HASH......${VER_SHA_HASH}"
echo "VERSION...........${VERSION}"
echo "APP_VERSION.......${APP_VERSION}"
echo "APP_VERSION_LONG..${APP_VERSION_LONG}"
echo "BUILD_DATE........${BUILD_DATE}"

SOURCE_DIR=lpub3d-${APP_VERSION}
echo "SOURCE_DIR........${SOURCE_DIR}"

echo "4. create cleaned tarball"
tar -czvf ../lpub3d_${APP_VERSION}.orig.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.travis.yml" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"
		
echo "5. create soruce directory ${SOURCE_DIR}"
cd ../
tar zxf lpub3d_${APP_VERSION}.orig.tar.gz
mv lpub3d ${SOURCE_DIR}

echo "6. get LDraw archive libraries"
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -N http://www.ldraw.org/library/unofficial/ldrawunf.zip
     mv ldrawunf.zip lpub3dldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -N http://www.ldraw.org/library/updates/complete.zip
fi

echo "7. copy debian configuration directory"
cp -rf ${SOURCE_DIR}/builds/linux/obs/debian ${SOURCE_DIR}
cd ${SOURCE_DIR}/debian

echo "8. build application package"
cd ../
chmod 755 debian/rules
/usr/bin/dpkg-buildpackage -us -uc

cd ../
DISTRO_FILE=`ls *.deb`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "9. create update and download packages"
    IFS=- read NAME_VERSION ARCH_EXTENSION <<< ${DISTRO_FILE}

    cp -rf ${DISTRO_FILE} "lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}"
    echo "    Download package: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
else
    echo "9. package ${DISTRO_FILE} not found"
fi

echo "$ME Finished!"
mv $LOG "${WORK_DIR}/debbuild/$ME.log"
