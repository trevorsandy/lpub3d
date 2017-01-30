#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreatePkg.sh
# $ ./CreatePkg.sh

LOG=`pwd`/CreatePkg.log
BUILD_DATE=`date "+%Y%m%d"`

echo "1. create PKG working directories" >> $LOG
if [ ! -d pkgbuild ]
then
    mkdir pkgbuild
    mkdir pkgbuild/upstream
else
   rm -rf pkgbuild
   mkdir pkgbuild
   mkdir pkgbuild/upstream
fi
cd pkgbuild/upstream

echo "2. download source" >> $LOG
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. capture version info" >> $LOG
if [ "$1" = "" ]
then
 PROJECT_VERSION=`cat "lpub3d/builds/utilities/version_info_unix"`
 IFS=- read VERSION REVISION BUILD SHA_HASH <<< ${PROJECT_VERSION}
 APP_VERSION=${VERSION}"."${BUILD}
 APP_VERSION_LONG=${VERSION}"."${REVISION}"."${BUILD}_${BUILD_DATE}
else
 APP_VERSION=$1
 APP_VERSION_LONG=$1"_"${BUILD_DATE}
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
cd ../

echo "5. copy PKGBUILD" >> $LOG
cp -f lpub3d/builds/linux/obs/PKGBUILD .

echo "6. update spec version" >> $LOG
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

echo "7. create package" >> $LOG
makepkg -s

cd ../PKG
DISTRO_FILE=`ls *.pkg.tar.xz`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    IFS=- read NAME VERSION BUILD ARCH_EXTENSION <<< ${DISTRO_FILE}

    echo "8. sign package" >> $LOG
    gpg --detach-sign ${DISTRO_FILE}

    echo "9. create update and download files" >> $LOG
    cp -f ${DISTRO_FILE} "lpub3d-${APP_VERSION_LONG}_${BUILD}_${ARCH_EXTENSION}"
    mv ${DISTRO_FILE} "UpdateMaster_${APP_VERSION}_${BUILD}_${ARCH_EXTENSION}"
    echo "Download file: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}" >> $LOG
    echo "  Update file: UpdateMaster_${APP_VERSION}_${ARCH_EXTENSION}" >> $LOG
else
    echo "8. package file not found." >> $LOG
fi

echo "Finished!" >> $LOG
mv ../CreateDeb.log CreateDeb.log
