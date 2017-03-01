#!/bin/bash
# Trevor SANDY
# Last Update 19 February 2017
# This script is automatically executed by qmake from mainApp.pro

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CHANGE_DATE_LONG=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`
CHANGE_DATE=`date +%a\ %b\ %d\ %Y`
DATE_TIME=`date +%d\ %m\ %Y\ %H:%M:%S`
BUILD_DATE=`date "+%Y%m%d"`
WORK_DIR=`pwd`

if [ "$1" = "" ]
then
        echo "Error: Did not receive required argument _PRO_FILE_PWD_"
        echo "$ME terminated!"
        exit 1
fi
PWD=$1

# logging stuff
LOG="$PWD/../builds/utilities/$ME.log"
if [ -f ${LOG} -a -r ${LOG} ]
then
        rm ${LOG}
fi
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start $ME execution..."
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
#         1 2  3  4   5       6
# format "2 0 20 17 663 410fdd7"
read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION  VER_BUILD VER_SHA_HASH <<< ${VERSION_INFO//'"'}
VERSION=${VER_MAJOR}"."${VER_MINOR}"."${VER_PATCH}
APP_VERSION=${VERSION}"."${VER_BUILD}
APP_VER_SUFFIX=${VER_MAJOR}${VER_MINOR}
APP_VERSION_LONG=${VERSION}"."${VER_REVISION}"."${VER_BUILD}_${BUILD_DATE}
BUILDVERSION=${VERSION}"."${VER_REVISION}"."${VER_BUILD}" ("${DATE_TIME}")"
#echo "   LOG...............${LOG}"
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
echo "   BUILDVERSION......${BUILDVERSION}"
echo "   DATE_TIME.........${DATE_TIME}"
echo "   CHANGE_DATE_LONG..${CHANGE_DATE_LONG}"

echo "2. update desktop configuration - add version suffix"
FILE="$PWD/lpub3d.desktop"
LineToReplace=10
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${LineToReplace}s/.*/Exec=lpub3d${APP_VER_SUFFIX} %f/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "3. update man page - add version suffix"
FILE="$PWD/docs/lpub3d.1"
LineToReplace=61
if [ -f ${FILE} -a -r ${FILE} ]
then  
        sed -i "${LineToReplace}s/.*/     \/usr\/bin\/lpub3d${APP_VER_SUFFIX}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "4. update PKGBUILD - add app version"
FILE="$PWD/../builds/linux/obs/PKGBUILD"
LineToReplace=3
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${LineToReplace}s/.*/pkgver=${APP_VERSION}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "5. create changelog - add app version and change date"
FILE="$PWD/../builds/linux/obs/debian/changelog"
if [ -f ${FILE} -a -r ${FILE} ]
then
	rm ${FILE}
fi
cat <<EOF >${FILE}
lpub3d (${APP_VERSION}) trusty; urgency=medium

  * LPub3D version ${APP_VERSION_LONG} for Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${CHANGE_DATE_LONG}
EOF

echo "6. update lpub3d.dsc - add app version"
FILE="$PWD/../builds/linux/obs/debian/lpub3d.dsc"
LineToReplace=5
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${LineToReplace}s/.*/Version: ${APP_VERSION}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "7. update readme.txt - add app version"
FILE="$PWD/docs/README.txt"
LineToReplace=1
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${LineToReplace}s/.*/LPub3D ${BUILDVERSION}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "8. update lpub3d.spec - add app version and change date"
FILE="$PWD/../builds/linux/obs/lpub3d.spec"
LineToReplace=222
if [ -f ${FILE} -a -r ${FILE} ]
then
        sed -i "${LineToReplace}s/.*/* ${CHANGE_DATE} - trevor.dot.sandy.at.gmail.dot.com ${APP_VERSION}/" "${FILE}"
else
        echo "Error: Cannot read ${FILE} from ${WORK_DIR}"
fi

echo "9. create source 'lpub3d.spec.git.version'"
FILE="$PWD/../builds/linux/obs/lpub3d.spec.git.version"
if [ -f ${FILE} -a -r ${FILE} ]
then
        rm ${FILE}
fi
cat <<EOF >${FILE}
${APP_VERSION}
EOF
echo "Script $ME execution finshed."
