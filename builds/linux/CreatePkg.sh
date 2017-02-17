#!/bin/bash
# Trevor SANDY
# Last Update 15 February 2017
# To run:
# $ chmod 755 CreatePkg.sh
# $ ./CreatePkg.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
BUILD_DATE=`date "+%Y%m%d"`

# logging stuff
LOG="${CWD}/$ME.log"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start $ME..."
echo "1. create PKG working directories"
if [ ! -d pkgbuild ]
then
    mkdir pkgbuild
    mkdir pkgbuild/upstream
fi
cd pkgbuild/upstream
WORK_DIR=lpub3d-git

echo "2. download source"
git clone https://github.com/trevorsandy/lpub3d.git
mv lpub3d ${WORK_DIR}

echo "3. update version info"
#         1 2  3  4   5       6    7  8  9       10
# format "2 0 20 17 663 410fdd7 2017 02 12 19:50:21"
FILE="${WORK_DIR}/builds/utilities/version.info"
if [ -f ${FILE} -a -r ${FILE} ]
then
    VERSION_INFO=`cat ${FILE}`
else
    echo "Error: Cannot read ${FILE} from `pwd`"
    echo "$ME terminated!"
    exit 1
fi
read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION VER_BUILD VER_SHA_HASH VER_REST <<< ${VERSION_INFO//'"'}
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

echo "4. create tarball"
tar -czvf ../${WORK_DIR}.tar.gz \
        --exclude="${WORK_DIR}/builds/linux/standard" \
        --exclude="${WORK_DIR}/builds/windows" \
        --exclude="${WORK_DIR}/builds/osx" \
        --exclude="${WORK_DIR}/lc_lib/tools" \
        --exclude="${WORK_DIR}/.travis.yml" \
        --exclude="${WORK_DIR}/.git" \
        --exclude="${WORK_DIR}/.gitattributes" \
        --exclude="${WORK_DIR}/LPub3D.pro.user" \
        --exclude="${WORK_DIR}/README.md" \
        --exclude="${WORK_DIR}/_config.yml" \
        --exclude="${WORK_DIR}/.gitignore" ${WORK_DIR}

echo "5. copy PKGBUILD"
cp -f ${WORK_DIR}/builds/linux/obs/PKGBUILD ../
cd ../

echo "6. get LDraw archive libraries"
if [ ! -f lpub3dldrawunf.zip ]
then
     wget -N -O lpub3dldrawunf.zip http://www.ldraw.org/library/unofficial/ldrawunf.zip
fi
if [ ! -f complete.zip ]
then
     wget -N http://www.ldraw.org/library/updates/complete.zip
fi

echo "7. build application package"
makepkg -s

DISTRO_FILE=`ls lpub3d-${APP_VERSION}*.pkg.tar.xz`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "8. create update and download packages"
    IFS=- read NAME VERSION BUILD ARCH_EXTENSION <<< ${DISTRO_FILE}
    cp -f ${DISTRO_FILE} "lpub3d-${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}"
    echo "    Download package: lpub3d_${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
else
    echo "8. package ${DISTRO_FILE} not found."
fi

echo "$ME Finished!"
mv $LOG "${CWD}/pkgbuild/$ME.log"
