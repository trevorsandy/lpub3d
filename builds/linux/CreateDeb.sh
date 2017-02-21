#!/bin/bash
# Trevor SANDY
# Last Update 14 February 2017
# To run:
# $ chmod 755 CreateDeb.sh
# $ ./CreateDeb.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
BUILD_DATE=`date "+%Y%m%d"`

# logging stuff
LOG="${CWD}/$ME.log"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start $ME..."
echo "1. create DEB working directories"
if [ ! -d debbuild ]
then
    mkdir debbuild
    mkdir debbuild/SOURCES
else
   rm -rf debbuild
   mkdir debbuild
   mkdir debbuild/SOURCES
fi
cd debbuild/SOURCES

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
    echo "Error: Cannot read ${FILE} from `pwd`"
    echo "$ME terminated!"
    exit 1
fi
read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION VER_BUILD VER_SHA_HASH <<< ${VERSION_INFO//'"'}
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
mv lpub3d ${SOURCE_DIR}

echo "4. create cleaned tarball lpub3d_${APP_VERSION}.orig.tar.gz using ${SOURCE_DIR}/"
tar -czvf ../lpub3d_${APP_VERSION}.orig.tar.gz ${SOURCE_DIR} \
        --exclude="${SOURCE_DIR}/builds/linux/standard" \
        --exclude="${SOURCE_DIR}/builds/osx" \
        --exclude="${SOURCE_DIR}/.travis.yml" \
        --exclude="${SOURCE_DIR}/.git" \
        --exclude="${SOURCE_DIR}/.gitattributes" \
        --exclude="${SOURCE_DIR}/LPub3D.pro.user" \
        --exclude="${SOURCE_DIR}/README.md" \
        --exclude="${SOURCE_DIR}/_config.yml" \
        --exclude="${SOURCE_DIR}/.gitignore"

echo "5. get LDraw archive libraries"
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -N -O lpub3dldrawunf.zip http://www.ldraw.org/library/unofficial/ldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -N http://www.ldraw.org/library/updates/complete.zip
fi

echo "6. create soruce directory ${SOURCE_DIR}"
cd ../
tar zxf lpub3d_${APP_VERSION}.orig.tar.gz

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
    IFS=_ read DEB_NAME DEB_VERSION DEB_ARCH_EXTENSION <<< ${DISTRO_FILE}

    cp -rf ${DISTRO_FILE} "lpub3d_${APP_VERSION_LONG}_${DEB_ARCH_EXTENSION}"
    echo "    Download package: lpub3d_${APP_VERSION_LONG}_${DEB_ARCH_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${DEB_ARCH_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${VERSION}_${DEB_ARCH_EXTENSION}"
else
    echo "9. package ${DISTRO_FILE} not found"
fi

echo "$ME Finished!"
mv $LOG "${CWD}/debbuild/$ME.log"
