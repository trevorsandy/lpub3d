#!/bin/bash
# Trevor SANDY
# Last Update January 24 2018
# To run:
# $ chmod 755 CreateDeb.sh
# $ [options] && ./builds/linux/CreateRpm.sh
# [options]:
#  - export DOCKER=true (if using Docker image)
#  - export OBS=false (if building locally)
# NOTE: elevated access required for apt-get install, execute with sudo
# or enable user with no password sudo if running noninteractive - see
# docker-compose/dockerfiles for script example of sudo, no password user.

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
LP3D_TARGET_ARCH=`uname -m`
export OBS=false # OpenSUSE Build Service flag must be set for CreateRenderers.sh - called by lpub3d.spec

echo "Start $ME execution at $CWD..."

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d}"
echo "   LPUB3D SOURCE DIR......${LPUB3D}"

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# logging stuff - increment log file name
f="${CWD}/$ME"
ext=".log"
if [[ -e "$f$ext" ]] ; then
  i=1
  f="${f%.*}";
  while [[ -e "${f}_${i}${ext}" ]]; do
    let i++
  done
  f="${f}_${i}${ext}"
else
  f="${f}${ext}"
fi
# output log file
LOG="$f"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "1. create RPM build working directories in rpmbuild/..."
if [ ! -d rpmbuild ]
then
    mkdir rpmbuild
fi

cd rpmbuild

BUILD_DIR=$PWD
for DIR in {BUILD,RPMS,SRPMS,SOURCES,SPECS}
do
    if [ ! -d "${DIR}" ]
    then
        mkdir "${DIR}"
    fi
done
cd ${BUILD_DIR}/SOURCES

echo "2. download ${LPUB3D} source to SOURCES/..."
git clone https://github.com/trevorsandy/${LPUB3D}.git

echo "3. source update_config_files.sh..."
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "4. move ${LPUB3D}/ to ${LPUB3D}-git/ in SOURCES/..."
WORK_DIR=${LPUB3D}-git
mv -f ${LPUB3D} ${WORK_DIR}

echo "6. copy lpub3d.xpm icon to SOURCES/"
cp -f ${WORK_DIR}/mainApp/images/lpub3d.xpm .

echo "7. copy ${LPUB3D}.spec to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec ${BUILD_DIR}/SPECS

echo "8. copy ${LPUB3D}-rpmlintrc to SPECS/"
cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}-rpmlintrc ${BUILD_DIR}/SPECS

echo "9. download LDraw archive libraries to SOURCES/..."
if [ ! -f lpub3dldrawunf.zip ]
then
    curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o lpub3dldrawunf.zip
fi
if [ ! -f complete.zip ]
then
    curl -O $curlopts http://www.ldraw.org/library/updates/complete.zip
fi

# file copy and downloads above must happen before we make the tarball
echo "11. create tarball ${WORK_DIR}.tar.gz from ${WORK_DIR}/..."
tar -czf ${WORK_DIR}.tar.gz \
        --exclude="${WORK_DIR}/builds/linux/standard" \
        --exclude="${WORK_DIR}/builds/windows" \
        --exclude="${WORK_DIR}/builds/macx" \
        --exclude="${WORK_DIR}/lclib/tools" \
        --exclude="${WORK_DIR}/.travis.yml" \
        --exclude="${WORK_DIR}/.gitattributes" \
        --exclude="${WORK_DIR}/LPub3D.pro.user" \
        --exclude="${WORK_DIR}/README.md" \
        --exclude="${WORK_DIR}/_config.yml" \
        --exclude="${WORK_DIR}/.gitignore" \
        --exclude="${WORK_DIR}/appveyor.yml" ${WORK_DIR}

cd ${BUILD_DIR}/SPECS
echo "12. Check ${LPUB3D}.spec..."
source /etc/os-release && if [ "$ID" = "fedora" ]; then sed 's/Icon: lpub3d.xpm/# Icon: lpub3d.xpm remarked - fedora does not like/' -i "${BUILD_DIR}/SPECS/${LPUB3D}.spec"; fi
rpmlint ${LPUB3D}.spec

echo "13. add ${LPUB3D} build dependencies [requires elevated access - sudo]..."
sudo dnf builddep -y ${LPUB3D}.spec

echo "14. build the RPM package..."
rpmbuild --define "_topdir ${BUILD_DIR}" -vv -bb ${LPUB3D}.spec

cd ${BUILD_DIR}/RPMS/${LP3D_TARGET_ARCH}
DISTRO_FILE=`ls ${LPUB3D}-${LP3D_APP_VERSION}*.rpm`
if [ -f ${DISTRO_FILE} ]
then
    echo "15-1. check rpm packages..."
    rpmlint ${DISTRO_FILE} ${LPUB3D}-${LP3D_APP_VERSION}*.rpm

    echo "15-2. create update and download packages..."
    RPM_EXTENSION="${DISTRO_FILE##*-}"

    cp -f ${DISTRO_FILE} "LPub3D-${LP3D_APP_VERSION_LONG}-${RPM_EXTENSION}"
    echo "    Download package..: LPub3D-${LP3D_APP_VERSION_LONG}-${RPM_EXTENSION}"

    mv -f ${DISTRO_FILE} "LPub3D-UpdateMaster_${LP3D_VERSION}-${RPM_EXTENSION}"
    echo "    Update package....: LPub3D-UpdateMaster_${LP3D_VERSION}-${RPM_EXTENSION}"
else
    echo "15. package ${DISTRO_FILE} not found."
fi

echo "16. cleanup cloned ${LPUB3D} repository from SOURCES/ and BUILD/..."
rm -rf ${BUILD_DIR}/SOURCES/${WORK_DIR} ${BUILD_DIR}/BUILD/${WORK_DIR}

# Elapsed execution time
FinishElapsedTime
