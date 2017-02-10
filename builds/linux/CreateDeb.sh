#!/bin/bash
# Trevor SANDY
# Last Update 10 February 2017
# To run:
# $ chmod 755 CreateDeb.sh
# $ ./CreateDeb.sh

BUILD_DATE=`date "+%Y%m%d"`
DATE_COMMAND=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`

# logging stuff
WORK_DIR=`pwd`
LOG="${WORK_DIR}/CreateDeb.log"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start..."
echo "1. create DEB working directories"
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

echo "2. download source"
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. capture version info"
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
SOURCE_DIR=lpub3d-${APP_VERSION}
echo "BUILD_DATE........${BUILD_DATE}"
echo "APP_VERSION.......${APP_VERSION}"
echo "APP_VERSION_LONG..${APP_VERSION_LONG}"
echo "SOURCE_DIR........${SOURCE_DIR}"
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
# - update desktop configuration - add version suffix
VPATTERN="{XX}"
SFILE="lpub3d/mainApp/lpub3d.desktop"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${VPATTERN}/${VER_SUFFIX}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi
# - update man page - add version suffix
SFILE="lpub3d/mainApp/lpub3d.1"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${VPATTERN}/${VER_SUFFIX}/g" "${SFILE}" > "${TFILE}" && mv "${TFILE}" "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi

echo "4. create cleaned tarball"
tar -czvf ../lpub3d_${APP_VERSION}.orig.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"
		
echo "5. create soruce directory ${SOURCE_DIR}"
cd ../
tar zxf lpub3d_${APP_VERSION}.orig.tar.gz
mv lpub3d ${SOURCE_DIR}

echo "6. get LDraw archive libraries" 
wget --directory-prefix=${SOURCE_DIR}/mainApp/extras http://www.ldraw.org/library/unofficial/ldrawunf.zip
mv ${SOURCE_DIR}/mainApp/extras/ldrawunf.zip ${SOURCE_DIR}/mainApp/extras/lpub3dldrawunf.zip
wget --directory-prefix=${SOURCE_DIR}/mainApp/extras http://www.ldraw.org/library/updates/complete.zip

echo "7. copy debian configuration directory"
cp -rf ${SOURCE_DIR}/builds/linux/obs/debian ${SOURCE_DIR}
cd ${SOURCE_DIR}/debian

echo "8. generate change log"
if [ -d changelog ]
then
	rm changelog
fi
cat <<EOF >changelog
lpub3d (${APP_VERSION}-0ubuntu1) trusty; urgency=medium

  * LPub3D (version ${APP_VERSION}) on Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${DATE_COMMAND}
EOF

echo "9. build application package"
cd ../
chmod 755 debian/rules
/usr/bin/dpkg-buildpackage -us -uc

cd ../
DISTRO_FILE=`ls *.deb`
if [ -f ${DISTRO_FILE} ] && [ ! -z ${DISTRO_FILE} ]
then
    echo "10. create update and download packages"
    IFS=- read NAME_VERSION ARCH_EXTENSION <<< ${DISTRO_FILE}

    cp -rf ${DISTRO_FILE} "lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}"
    echo "    Download package: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}"

    mv ${DISTRO_FILE} "LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
    echo "      Update package: LPub3D-UpdateMaster_${VERSION}_${ARCH_EXTENSION}"
else
    echo "10. package ${DISTRO_FILE} not found"
fi

echo "Finished!"
mv $LOG "${WORK_DIR}/debbuild/CreateDeb.log"
