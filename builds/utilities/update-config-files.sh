#!/bin/bash
# Trevor SANDY
# Last Update 14 February 2017
# This script is automatically executed by qmake from mainApp.pro

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
BUILD_DATE=`date "+%Y%m%d"`
CHANGE_DATE=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`
WORK_DIR=`pwd`

# updated line numbers - change if you modify respective file
desktopLn1=10
manPageLn1=61
pkgBuildLn1=3
debDscLn1=5
rpmSpecLn1=unknown

if [ "$1" = "" ]
then
        echo "Error: Did not receive required argument _PRO_FILE_PWD_"
        echo "$ME terminated!"
        exit 1
fi
PWD=$1
# logging stuff
LOG="$PWD/../builds/utilities/update-config-files.log"
if [ -f ${LOG} -a -r ${LOG} ]
then
        rm ${LOG}
fi
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start $ME..."
echo "1. capture version info"
FILE="$PWD/../builds/utilities/version.info"
if [ -f ${FILE} -a -r ${FILE} ]
then
	VERSION_INFO=`cat ${FILE}`
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
        echo "$ME terminated!"
	exit 1
fi
#         1 2  3  4   5       6    7  8  9       10
# format "2 0 20 17 663 410fdd7 2017 02 12 19:50:21"
read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION  VER_BUILD VER_SHA_HASH VER_REST <<< ${VERSION_INFO//'"'}
VERSION=${VER_MAJOR}"."${VER_MINOR}"."${VER_PATCH}
APP_VERSION=${VERSION}"."${VER_BUILD}
APP_VER_SUFFIX=${VER_MAJOR}${VER_MINOR}
APP_VERSION_LONG=${VERSION}"."${VER_REVISION}"."${VER_BUILD}_${BUILD_DATE}
echo "   LOG...............${LOG}"
echo "   WORK_DIR..........${WORK_DIR}"
echo "   VER_MAJOR.........${VER_MAJOR}"
echo "   VER_MINOR.........${VER_MINOR}"
echo "   VER_PATCH.........${VER_PATCH}"
echo "   VER_REVISION......${VER_REVISION}"
echo "   VER_BUILD.........${VER_BUILD}"
echo "   VER_SHA_HASH......${VER_SHA_HASH}"
echo "   VERSION...........${VERSION}"
echo "   APP_VERSION.......${APP_VERSION}"
echo "   APP_VER_SUFFIX....${APP_VER_SUFFIX}"
echo "   APP_VERSION_LONG..${APP_VERSION_LONG}"
echo "   BUILD_DATE........${BUILD_DATE}"
echo "   CHANGE_DATE.......${CHANGE_DATE}"

echo "2. update desktop configuration - add version suffix"
FILE="$PWD/lpub3d.desktop"
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${desktopLn1}s/.*/Exec=lpub3d${APP_VER_SUFFIX} %f/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "3. update man page - add version suffix"
FILE="$PWD/lpub3d.1"
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${manPageLn1}s/.*/     \/usr\/bin\/lpub3d${APP_VER_SUFFIX}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "4. update PKGBUILD - add version"
FILE="$PWD/../builds/linux/obs/PKGBUILD"
if [ -f ${FILE} -a -r ${FILE} ] || [ -f ${SBFILE=} -a -r ${SBFILE} ]
then
        sed -i "${pkgBuildLn1}s/.*/pkgver=${APP_VERSION}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "5. create changelog - add version and change date"
FILE="$PWD/../builds/linux/obs/debian/changelog"
if [ -f ${FILE} -a -r ${FILE} ]
then
	rm ${FILE}
fi
cat <<EOF >${FILE}
lpub3d (${APP_VERSION}) trusty; urgency=medium

  * LPub3D version ${APP_VERSION_LONG} for Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${CHANGE_DATE}
EOF

echo "6. update lpub3d.dsc - add version"
FILE="$PWD/../builds/linux/obs/debian/lpub3d.dsc"
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${debDscLn1}s/.*/Version: ${APP_VERSION}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "7. create source 'lpub3d.spec.git.version'"
FILE="$PWD/../builds/linux/obs/lpub3d.spec.git.version"
if [ -f ${FILE} -a -r ${FILE} ]
then
        rm ${FILE}
fi
cat <<EOF >${FILE}
${APP_VERSION}
EOF
