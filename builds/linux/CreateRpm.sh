#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreateRpm.sh
# $ ./CreateRpm.sh

LOG=`pwd`/CreateRpm.log
BUILD_DATE=`date "+%Y%m%d"`

echo "1. create RPM build working directories" >> $LOG
if [ ! -d rpmbuild ]
then
    mkdir rpmbuild
else
    rm -rf rpmbuild
    mkdir rpmbuild
fi
cd rpmbuild
for i in {BUILD,RPMS,SOURCES,SPECS,SRPMS,tmp}
    if [ ! -d "$i" ]
    then
        mkdir "$i"
    fi
done
cd rpmbuild/SOURCES

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
tar -czvf lpub3d.git.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"

echo "5. copy xpm icon to SOURCES/" >> $LOG
cp -f lpub3d/mainApp/images/lpub3d.xpm .

echo "6. copy spec to SPECS/" >> $LOG
cp -f lpub3d/builds/linux/obs/lpub3d.spec ../SPECS/

echo "7. remove cloned repository from SOURCES/" >> $LOG
rm -rf lpub3d

echo "8. update spec version" >> $LOG
cd ../SPECS
OLD="{X.XX.XX.XXX}"
SFILE="lpub3d.spec"
TFILE="/tmp/out.tmp.$$"
if [ -f ${SFILE} -a -r ${SFILE} ]
then
    sed "s/${OLD}/${APP_VERSION}/g" "${SFILE}" > ${TFILE} && mv ${TFILE} "${SFILE}"
else
    echo "Error: Cannot read ${SFILE}"
fi
rm ${TFILE}

echo "9. build and sign the RPM package (success = 'exit 0')" >> $LOG
rpmbuild -v -ba --sign lpub3d.spec

cd ../RPMS/x86_64
DISTRO_FILE=`ls *.rpm`
if [ -f ${DISTRO_FILE} ] && [ -z ${DISTRO_FILE} ]
then
    echo "10. create update and download files" >> $LOG
    IFS=- read NAME VERSION ARCH_EXTENSION <<< ${DISTRO_FILE}
    cp -f ${DISTRO_FILE} "lpub3d-${APP_VERSION_LONG}_${ARCH_EXTENSION}"
    mv ${DISTRO_FILE} "UpdateMaster_${APP_VERSION}_${ARCH_EXTENSION}"
    echo "Download file: lpub3d_${APP_VERSION_LONG}_${ARCH_EXTENSION}" >> $LOG
    echo "  Update file: UpdateMaster_${APP_VERSION}_${ARCH_EXTENSION}" >> $LOG
else
    echo "10. package file not found." >> $LOG
fi

echo "Finished!" >> $LOG
mv ../CreateDeb.log CreateDeb.log
