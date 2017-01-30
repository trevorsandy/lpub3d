#!/bin/bash
# Trevor SANDY
# Last Update 28 January 2017
# To run:
# $ chmod 755 CreateDeb.sh
# $ ./CreateDeb.sh

LOG=CreateDeb.log
WORK_DIR=`pwd`
BUILD_DATE=`date "+%Y%m%d"`

if [ "$1" = "" ]
then
 PROJECT_VERSION=`cat ../utilities/version_info_unix`
 IFS=- read VERSION REVISION BUILD SHA_HASH <<< ${PROJECT_VERSION}
 APP_VERSION=${VERSION}"."${BUILD}
 APP_VERSION_LONG=${VERSION}"."${REVISION}"."${BUILD}_${BUILD_DATE}
else
 APP_VERSION=UpdateMaster"_"$1
 APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi

echo "1. create DEB working directories" >> $LOG
if [ ! -d debbuild ]
then
    mkdir debbuild
    mkdir debbuild/upstream
fi
cd debbuild/upstream

echo "2. download source" >> $LOG
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. create tarball" >> $LOG
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

echo "4. create package config: Name, Version, Path to tarball" >> $LOG
/usr/bin/bzr dh-make lpub3d ${APP_VERSION} upstream/lpub3d.git.tar.gz

echo "5. delete unneeded config files" >> $LOG
cd lpub3d/debian
rm *ex *EX changelog

echo "6. copy standard config files" >> $LOG
cp -rf ../../upstream/lpub3d/builds/linux/obs/debian/* .

echo "7. update change log" >> $LOG
DATE_COMMAND=`date "+%a,\ %b\ %d\ %Y\ %H:%M:%S\ +hhmm"`
cat <<EOF >changelog
lpub3d (${APP_VERSION}-0ubuntu1) trusty; urgency=medium

  * LPub3D (version ${APP_VERSION}) on Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${DATE_COMMAND}
EOF
cd ${WORK_DIR}

echo "8. add format to package and commit package source" >> $LOG
/usr/bin/bzr add source/format
/usr/bin/bzr commit -m "Packaging commit for lpub3d ${APP_VERSION}."

echo "9. build and sign application" >> $LOG
bzr builddeb -S

echo "10. create update and download files" >> $LOG
DISTRO_FILE=`find -name "*.deb"`
if [ -f ${DISTRO_FILE} ]
then
    IFS=- read NAME_VERSION ARCH_EXTENSION <<< ${DISTRO_FILE}
    cp -rf ${DISTRO_FILE} "lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}"
    mv ${DISTRO_FILE} "UpdateMaster_${APP_VERSION}_${ARCH_EXTENSION}"
    echo "  Update file: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}" >> $LOG
    echo "Download file: UpdateMaster_${APP_VERSION}_${ARCH_EXTENSION}" >> $LOG
fi

echo "Finished!" >> $LOG
