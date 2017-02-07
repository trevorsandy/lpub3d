#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreateRpm.sh
# $ ./CreateRpm.sh

WORK_DIR=`pwd`
BUILD_DATE=`date "+%Y%m%d"`
CHANGE_DATE=`date "+%a %b %d %Y"`
# logging stuff
LOG="${WORK_DIR}/CreateRpm.log"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start..."
echo "1. create RPM build working directories"
if [ ! -d rpmbuild ]
then
    mkdir rpmbuild
fi
cd rpmbuild
for i in {BUILD,RPMS,SRPMS,SOURCES,SPECS}
do
    if [ ! -d "$i" ]
    then
        mkdir "$i"
    fi
done
cd SOURCES

echo "2. download source"
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. update version info"
# - get version info
if [ "$1" = "" ]
then
 PROJECT_VERSION=`cat "lpub3d/builds/utilities/version_info_posix"`
 IFS=- read VERSION REVISION BUILD SHA_HASH <<< ${PROJECT_VERSION//'"'}
 APP_VERSION=${VERSION}"."${BUILD}
 APP_VERSION_LONG=${VERSION}"."${REVISION}"."${BUILD}_${BUILD_DATE}
 echo "VERSION...........${VERSION}"
 echo "REVISION..........${REVISION}"
 echo "BUILD.............${BUILD}"
 echo "SHA_HASH..........${SHA_HASH}"
else
 VERSION=$1
 APP_VERSION=$1
 APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi
echo "BUILD_DATE........${BUILD_DATE}"
echo "APP_VERSION.......${APP_VERSION}"
echo "APP_VERSION_LONG..${APP_VERSION_LONG}"
# - populate VER_SUFFIX variable
if [ ! ${APP_VERSION} = "" ]
then
 IFS=. read VER_MAJOR VER_MINOR VER_PACK VER_BUILD <<< ${APP_VERSION}
 VER_SUFFIX=${VER_MAJOR}${VER_MINOR}
 echo "VER_MAJOR.........${VER_MAJOR}"
 echo "VER_MINOR.........${VER_MINOR}"
 echo "VER_PACK..........${VER_PACK}"
 echo "VER_BUILD.........${VER_BUILD}"
 echo "VER_SUFFIX........${VER_SUFFIX}"
fi
# - update desktop configuration file with version number
VPATTERN="{XX}"
SFILE="lpub3d/mainApp/lpub3d.desktop"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${VPATTERN}/${VER_SUFFIX}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi
# - update lpub3d.1 manpage template with version number
SFILE="lpub3d/mainApp/lpub3d.1"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${VPATTERN}/${VER_SUFFIX}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi

echo "4. create tarball"
tar -czvf lpub3d.git.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"

echo "5. copy xpm icon to SOURCES/"
cp -f lpub3d/mainApp/images/lpub3d.xpm .

echo "6. copy spec to SPECS/"
cp -f lpub3d/builds/linux/obs/lpub3d.spec ../SPECS/

echo "7. remove cloned repository from SOURCES/"
#rm -rf lpub3d

echo "8. update spec version and date time"
cd ../SPECS
VPATTERN="{X.XX.XX.XXX}"
SFILE="lpub3d.spec"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
	sed "s/${VPATTERN}/${APP_VERSION}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi

echo "9. build the RPM package (success = 'exit 0')"
rpmbuild --define "_topdir ${WORK_DIR}/rpmbuild" -v -ba lpub3d.spec

cd ../RPMS/x86_64
DISTRO_FILE=`find -name "lpub3d-${APP_VERSION}*.rpm"`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "10. create update and download files"
    IFS=- read NAME VERSION ARCH_EXTENSION <<< ${DISTRO_FILE}

    cp -f ${DISTRO_FILE} "lpub3d-${APP_VERSION_LONG}_${ARCH_EXTENSION}"
    echo "    Download file: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "      Update file: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
else
    echo "10. package file not found."
fi

echo "Finished!"
mv $LOG "${WORK_DIR}/rpmbuild/CreateRpm.log"
