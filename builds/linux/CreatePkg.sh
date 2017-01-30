#!/bin/bash
# Trevor SANDY
# Last Update 28 January 2017
# To run:
# $ chmod 755 CreatePkg.sh
# $ ./CreatePkg.sh

LOG=CreatePkg.log
WORK_DIR=`pwd`
BUILD_DATE=`date "+%Y%m%d"`

if [ "$1" = "" ]
then
 PROJECT_VERSION=`cat ../utilities/version_info_unix`
 IFS=- read VERSION REVISION BUILD SHA_HASH <<< ${PROJECT_VERSION}
 APP_VERSION=UpdateMaster_${VERSION}"."${BUILD}
 APP_VERSION_LONG=${VERSION}"."${REVISION}"."${BUILD}_${BUILD_DATE}
else
 APP_VERSION=UpdateMaster"_"$1
 APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi

echo "1. create PKG working directories" >> $LOG
if [ ! -d pkgbuild ]
then
    mkdir pkgbuild
    mkdir pkgbuild/upstream
fi
cd pkgbuild/upstream

echo "2. download source" >> $LOG
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. create tarball" >> $LOG
tar -czvf ../lpub3d.git.tar.gz \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore" lpub3d
cd ../

echo "4. copy PKGBUILD" >> $LOG
cp -f lpub3d/builds/linux/obs/PKGBUILD .

echo "5. update spec version" >> $LOG
cd ../SPECS
OLD="{X.XX.XX.XXX}"
SFILE="PKGBUILD"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${OLD}/${APP_VERSION}/g" "${SFILE}" > ${TFILE} && mv ${TFILE} "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi
rm ${TFILE}

echo "6. create package" >> $LOG
makepkg -s

cd ../PKG
DISTRO_FILE=`find -name "*.pkg.tar.xz"`
if [ -f ${DISTRO_FILE} ]
then
    IFS=- read NAME VERSION BUILD ARCH_EXTENSION <<< ${DISTRO_FILE}

    echo "7. sign package" >> $LOG
    gpg --detach-sign ${DISTRO_FILE}

    echo "8. create update and download files" >> $LOG
    cp -f ${DISTRO_FILE} "lpub3d-${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}"
    mv ${DISTRO_FILE} "UpdateMaster_${APP_VERSION}_${BUILD}_${ARCH_EXTENSION}"
    echo "  Update file: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}" >> $LOG
    echo "Download file: UpdateMaster_${APP_VERSION}_${ARCH_EXTENSION}" >> $LOG
fi

echo "Finished!" >> $LOG
