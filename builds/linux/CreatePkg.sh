#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreatePkg.sh
# $ ./CreatePkg.sh

WORK_DIR=`pwd`
LOG="${WORK_DIR}/CreatePkg.log"
BUILD_DATE=`date "+%Y%m%d"`

echo "1. create PKG working directories" >> $LOG
if [ ! -d pkgbuild ]
then
    mkdir pkgbuild
    mkdir pkgbuild/upstream
fi
cd pkgbuild/upstream

echo "2. download source" >> $LOG
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. update version info" >> $LOG
if [ "$1" = "" ]
then
 PROJECT_VERSION=`cat "lpub3d/builds/utilities/version_info_posix"`
 IFS=- read VERSION REVISION BUILD SHA_HASH <<< ${PROJECT_VERSION//'"'}
 APP_VERSION=${VERSION}"."${BUILD}
 APP_VERSION_LONG=${VERSION}"."${REVISION}"."${BUILD}_${BUILD_DATE}
else
 VERSION=$1
 APP_VERSION=$1
 APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi
if [ ! ${APP_VERSION} = "" ]
then
 IFS=. read VER_MAJOR VER_MINOR <<< ${APP_VERSION}
 APP_VER=${VER_MAJOR}${VER_MINOR}
fi
VPATTERN="{XX}"
SFILE="lpub3d/mainApp/lpub3d.desktop"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${VPATTERN}/${APP_VER}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi
SFILE="lpub3d/mainApp/lpub3d.1"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${VPATTERN}/${APP_VER}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi

echo "4. create tarball" >> $LOG
tar -czvf ../lpub3d.git.tar.gz \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore" lpub3d

echo "5. copy PKGBUILD" >> $LOG
cp -f lpub3d/builds/linux/obs/PKGBUILD ../

echo "6. update PKGBUILD version" >> $LOG
cd ../
VPATTERN="{X.XX.XX.XXX}"
SFILE="PKGBUILD"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${VPATTERN}/${APP_VERSION}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi

echo "7. create package" >> $LOG
makepkg -s

DISTRO_FILE=`ls lpub3d-${APP_VERSION}*.pkg.tar.xz`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "8. create update and download files" >> $LOG
    IFS=- read NAME VERSION BUILD ARCH_EXTENSION <<< ${DISTRO_FILE}
    cp -f ${DISTRO_FILE} "lpub3d-${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}"
    echo "    Download file: lpub3d_${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}" >> $LOG

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "      Update file: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}" >> $LOG
else
    echo "8. package file not found." >> $LOG
fi

echo "Finished!" >> $LOG
mv $LOG "${WORK_DIR}/pkgbuild/CreatePkg.log"
