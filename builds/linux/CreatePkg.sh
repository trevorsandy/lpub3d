#!/bin/bash
# Trevor SANDY
# Last Update June 25, 2019
# Copyright (c) 2017 - 2019 by Trevor SANDY
# To run:
# $ chmod 755 CreateDeb.sh
# $ [options] && ./builds/linux/CreatePkg.sh
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
export OBS=false # OpenSUSE Build Service flag must be set for CreateRenderers.sh - called by PKGBUILD

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

echo "1. create PKG working directories in pkgbuild/"
if [ ! -d pkgbuild/upstream ]
then
  mkdir -p pkgbuild/upstream
fi

cd pkgbuild/upstream

echo "2. download ${LPUB3D}/ to upstream/"
git clone https://github.com/trevorsandy/${LPUB3D}.git

# For Docker build, check if there is a tag after the last commit
if [ "$DOCKER" = "true" ]; then
   # Setup git command
   GIT_CMD="git --git-dir $PWD/${LPUB3D}/.git --work-tree $PWD/${LPUB3D}"
   #1. Get the latest version tag - check across all branches
   BUILD_TAG=$($GIT_CMD describe --tags --match v* $($GIT_CMD rev-list --tags --max-count=1) 2> /dev/null)
   if [ -n "$BUILD_TAG" ]; then
       #2. Get the tag datetime
       BUILD_TAG_TIME=$($GIT_CMD log -1 --format=%ai $BUILD_TAG 2> /dev/null)
       #3. Get the latest commit datetime from the build branch
       GIT_COMMIT_TIME=$($GIT_CMD log -1 --format=%ai 2> /dev/null)
           #4. If tag is newer than commit, check out the tag
       if [ $(date -d "$GIT_COMMIT_TIME" +%s) -lt $(date -d "$BUILD_TAG_TIME" +%s) ]; then
           echo "2a. checking out build tag $BUILD_TAG..."
           $GIT_CMD checkout -qf $BUILD_TAG
       fi
   fi
fi

_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

echo "3. move ${LPUB3D}/ to ${LPUB3D}-git/ in upstream/"
WORK_DIR=${LPUB3D}-git
mv -f ${LPUB3D} ${WORK_DIR}

echo "4. create tarball ${WORK_DIR}.tar.gz from ${WORK_DIR}/"
tar -czf ../${WORK_DIR}.tar.gz \
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

echo "5. copy PKGBUILD"
cp -f ${WORK_DIR}/builds/linux/obs/PKGBUILD ../
cd ../

echo "6. download LDraw archive libraries to pkgbuild/"
if [ ! -f lpub3dldrawunf.zip ]
then
    curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o lpub3dldrawunf.zip
fi
if [ ! -f complete.zip ]
then
    curl -O $curlopts http://www.ldraw.org/library/updates/complete.zip
fi
if [ ! -f tenteparts.zip ]
then
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip
fi
if [ ! -f vexiqparts.zip ]
then
    curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip
fi

if [ ! -f mesa-18.3.5.tar.gz ]
then
  curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/mesa-18.3.5.tar.gz
fi

if [ ! -f glu-9.0.0.tar.bz2 ]
then
  curl -O $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/glu-9.0.0.tar.bz2
fi

# download 3rd party packages defined as source in PKGBUILD
echo "7. copy 3rd party source to pkgbuild/"
for buildDir in ldglite ldview povray; do
  case ${buildDir} in
  ldglite)
    curlCommand="https://github.com/trevorsandy/ldglite/archive/master.tar.gz"
    ;;
  ldview)
    curlCommand="https://github.com/trevorsandy/ldview/archive/qmake-build.tar.gz"
    ;;
  povray)
    curlCommand="https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.tar.gz"
    ;;
  esac
  if [ ! -f ${buildDir}.tar.gz ]; then
    Info "`echo ${buildDir} | awk '{print toupper($0)}'` tarball ${buildDir}.tar.gz does not exist. Downloading..."
    curl $curlopts -o ${buildDir}.tar.gz ${curlCommand}
  fi
done

echo "8. build application package"
makepkg --syncdeps --noconfirm --needed

DISTRO_FILE=`ls ${LPUB3D}-${LP3D_APP_VERSION}*.pkg.tar.xz`
if [ -f ${DISTRO_FILE} ]
then
    echo "9-1. Build-check ${DISTRO_FILE}"
    if [ ! -f "/usr/bin/update-desktop-database" ]; then
        echo "      Program update-desktop-database not found. Installing..."
        sudo pacman -S --noconfirm --needed desktop-file-utils
    fi
    # Install package - here we use the distro file name e.g. LPub3D-2.3.8.1566-1-x86_64.pkg.tar.xz
    echo "      9-1. Build-check install ${LPUB3D}..."
    sudo pacman -U --noconfirm ${DISTRO_FILE}
    # Check if exe exist - here we use the executable name e.g. lpub3d22
    LPUB3D_EXE=lpub3d${LP3D_APP_VER_SUFFIX}
    SOURCE_DIR=src/${WORK_DIR}
    if [ -f "/usr/bin/${LPUB3D_EXE}" ]; then
        # Check commands
        source ${SOURCE_DIR}/builds/check/build_checks.sh
        echo "      9-1. Build-check uninstall ${LPUB3D}..."
        # Cleanup - here we use the package name e.g. lpub3d
        sudo pacman -Rs --noconfirm ${LPUB3D}
    else
        echo "9-1. Build-check failed - /usr/bin/${LPUB3D_EXE} not found."
    fi

    echo "9-2. create update and download packages"
    IFS=- read PKG_NAME PKG_VERSION BUILD PKG_EXTENSION <<< ${DISTRO_FILE}

    mv -f ${DISTRO_FILE} "LPub3D-${LP3D_APP_VERSION_LONG}-${PKG_EXTENSION}"
    if [ -f "LPub3D-${LP3D_APP_VERSION_LONG}-${PKG_EXTENSION}" ]; then
        echo "    Download package..: LPub3D-${LP3D_APP_VERSION_LONG}-${PKG_EXTENSION}"
    else
        echo "    ERROR - file not copied: LPub3D-${LP3D_APP_VERSION_LONG}-${PKG_EXTENSION}"
    fi

else
    echo "9. package ${DISTRO_FILE} not found."
fi

# Elapsed execution time
FinishElapsedTime
