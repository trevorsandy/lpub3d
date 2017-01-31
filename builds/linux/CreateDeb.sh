#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreateDeb.sh
# $ ./CreateDeb.sh

WORK_DIR=`pwd`
LOG="${WORK_DIR}/CreateDeb.log"
BUILD_DATE=`date "+%Y%m%d"`

echo "1. create DEB working directories" >> $LOG
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
 VERSION=$1
 APP_VERSION=$1
 APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi

echo "4. create tarball" >> $LOG
tar -czvf lpub3d.git.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"
cd ../

echo "5. create package config: Name, Version, Path to tarball" >> $LOG
/usr/bin/bzr dh-make lpub3d ${APP_VERSION} upstream/lpub3d.git.tar.gz

echo "6. delete unneeded config files" >> $LOG
cd lpub3d/debian
rm *ex *EX changelog

echo "7. copy standard config files" >> $LOG
cp -rf ../../upstream/lpub3d/builds/linux/obs/debian/* .

echo "8. update change log" >> $LOG
DATE_COMMAND=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`
cat <<EOF >changelog
lpub3d (${APP_VERSION}-0ubuntu1) trusty; urgency=medium

  * LPub3D (version ${APP_VERSION}) on Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${DATE_COMMAND}
EOF

echo "9. add format to package and commit package source" >> $LOG
/usr/bin/bzr add source/format
#/usr/bin/bzr commit -m "Packaging commit for lpub3d ${APP_VERSION}."

echo "10. build and sign application" >> $LOG
/usr/bin/bzr builddeb -- -us -uc
# sign package - can be removed if signing is not needed
/usr/bin/bzr builddeb -S

cd ../../
DISTRO_FILE=`ls *.deb`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "11. create update and download files" >> $LOG
    IFS=- read NAME_VERSION ARCH_EXTENSION <<< ${DISTRO_FILE}

    cp -rf ${DISTRO_FILE} "lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}"
    echo "Download file: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}" >> $LOG

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "  Update file: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}" >> $LOG
else
    echo "11. package file not found." >> $LOG
fi

echo "Finished!" >> $LOG
mv $LOG "${WORK_DIR}/CreateDeb.log"
