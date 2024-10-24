#!/bin/bash
# Trevor SANDY
# Last Update October 21, 2024
#
# This script is called from .github/workflows/codeql.yml
#
# Run command:
# bash -ex builds/utilities/ci/github/linux-compile.sh

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  if [ -z "${LP3D_ANALYZE}" ]; then
    echo "Compile Renderers Finished!"
  else
    echo "${ME} Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

FinishRenderers () {
  if [ -d "$WD" ]; then
    rm -rf "$WD"
  fi
  FinishElapsedTime
}

# check if build is on stale commit
curl -s -H "Authorization: Bearer ${GITHUB_TOKEN}" https://api.github.com/repos/${GITHUB_REPOSITORY}/commits/master -o repo.txt
LP3D_REMOTE=$(cat repo.txt | jq -r '.sha')
LP3D_LOCAL=$(git rev-parse HEAD)
if [[ "$LP3D_REMOTE" != "$LP3D_LOCAL" ]]; then
  echo "WARNING - Build no longer current. Rmote: '$LP3D_REMOTE', Local: '$LP3D_LOCAL' - aborting build."
  [ -f "repo.txt" ] && echo "Repo response:" && cat repo.txt || :
  exit 0
fi

# Grab the script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# Set working directory
if [ -z "${LP3D_ANALYZE}" ]; then
  # Build in a temporary directory, use RAM disk if possible
  if [ -d /dev/shm ] && mount | grep /dev/shm | grep -v -q noexec; then
    TEMP_BASE=/dev/shm
  elif [ -d /docker-ramdisk ]; then
    TEMP_BASE=/docker-ramdisk
  else
    TEMP_BASE=/tmp
  fi

  export WD="$(mktemp -d -p "$TEMP_BASE" build-XXXXXX)"

  cp -Rf ${GITHUB_WORKSPACE}/. $WD/

  trap FinishRenderers EXIT
else
  # Make sure we're in the repository root directory
  export WD="${GITHUB_WORKSPACE}"

  trap FinishElapsedTime EXIT
fi

# Move to working directory
cd $WD

# Prepare output directory
LP3D_OUT_PATH="${LP3D_BUILDPKG_PATH}"
[ ! -d "${LP3D_OUT_PATH}" ] && \
mkdir -p ${LP3D_OUT_PATH} || :

# Prepare exported variables
oldIFS=$IFS; IFS='/' read -ra LP3D_SLUGS <<< "${GITHUB_REPOSITORY}"; IFS=$oldIFS;

export LPUB3D=${LPUB3D:-${LP3D_SLUGS[1]}}
export LP3D_ARCH=${LP3D_ARCH:-amd64}
export LP3D_BASE=${LP3D_BASE:-ubuntu}
export LP3D_QEMU=${LP3D_QEMU:-false}
export LP3D_APPIMAGE=${LP3D_APPIMAGE:-false}

export LP3D_DIST_DIR_PATH=${LP3D_DIST_DIR_PATH:-${LP3D_3RD_PARTY_PATH}/${LP3D_BASE}_${LP3D_ARCH}}
export LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR:-lpub3d_linux_3rdparty}
export LP3D_LOG_PATH=${LP3D_LOG_PATH:-${LP3D_OUT_PATH}}
export LP3D_TD=${WD}/${LP3D_3RD_DIST_DIR}

export LP3D_LDRAW_DIR=${LP3D_LDRAW_DIR:-${LP3D_3RD_PARTY_PATH}/ldraw}
export LDRAWDIR_ROOT=${LDRAWDIR_ROOT:-~}
export LDRAWDIR=${LDRAWDIR:-~/ldraw}

export LP3D_VER_LDGLITE=ldglite-1.3
export LP3D_VER_LDVIEW=ldview-4.4
export LP3D_VER_POVRAY=lpub3d_trace_cui-3.8

export CI=${CI:-true}
export OBS=${OBS:-false}
export DOCKER=${DOCKER:-false}
export GITHUB=${GITHUB:-true}

export LP3D_NO_DEPS=${LP3D_NO_DEPS:-true}
export LP3D_NO_CLEANUP=${LP3D_NO_CLEANUP:-true}

set +x && echo
echo "SOURCE DIR.........${LPUB3D}"
echo "BUILD BASE.........${LP3D_BASE}"
echo "BUILD ARCH.........${LP3D_ARCH}"
echo "QEMU...............${LP3D_QEMU}"
echo "DOCKER.............${DOCKER}"
echo "APPIMAGE...........${LP3D_APPIMAGE}"
echo "WORKING DIRECTORY..${WD}"
echo "DISTRIBUTION PATH..${LP3D_DIST_DIR_PATH}"
echo "LDRAW DIRECTORY....${LDRAWDIR}"
echo "LOG_PATH...........${LP3D_LOG_PATH}"
echo && set -x

# Output logs
f="${0##*/}"; f="${f%.*}"; f="${f}-${LP3D_BASE}-${LP3D_ARCH}"
f="${LP3D_LOG_PATH}/${f}"
l=${f}_libs
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
LOG="$f"
LIBS_LOG="${l}.log"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

# Link lpub3d_linux_3rdparty to package build source path
if [[ ! -L "${LP3D_TD}" && ! -d "${LP3D_TD}" ]]; then
  ln -sf "${LP3D_DIST_DIR_PATH}" "${LP3D_3RD_DIST_DIR}"
  if [ -L "${LP3D_3RD_DIST_DIR}" ]; then
    echo "${LP3D_DIST_DIR_PATH} linked to ${LP3D_TD}"
  else
    echo "ERROR - ${LP3D_DIST_DIR_PATH} failed to link to ${LP3D_3RD_DIST_DIR}"
    exit 4
  fi
else
  echo "Directory ${LP3D_TD} exists. Nothing to do."
fi

# Setup for analysis
if [[ -z "${LP3D_ANALYZE}" || (-n "${LP3D_ANALYZE}" && "${LP3D_ANALYZE}" -gt "1") ]]; then
  # Required libraries
  case "${LP3D_BASE}" in
    "ubuntu")
      cp -f builds/linux/obs/alldeps/debian/control .
      sudo apt-get update > ${LIBS_LOG} 2>&1
      sudo apt-get install -y apt-utils cmake rsync unzip lintian build-essential fakeroot ccache lsb-release >> ${LIBS_LOG} 2>&1
      sudo apt-get install -y autotools-dev autoconf pkg-config automake libtool curl zip >> ${LIBS_LOG} 2>&1
      sudo apt-get install -y $(grep Build-Depends control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,) >> ${LIBS_LOG} 2>&1
      ;;
    "fedora")
      cp -f builds/linux/obs/alldeps/lpub3d.spec .
      sed -e 's/Icon: lpub3d.xpm/# Icon: lpub3d.xpm remarked - fedora does not like/' \
          -e 's/<B_CNT>/1/' -i lpub3d.spec
      sudo dnf install -y git wget unzip rsync which rpmlint ccache dnf-plugins-core > ${LIBS_LOG} 2>&1
      sudo dnf builddep -y ./lpub3d.spec >> ${LIBS_LOG} 2>&1
      ;;
    "archlinux")
      cp -f builds/linux/obs/alldeps/PKGBUILD .
      sudo pacman -Sy --noconfirm git wget unzip rsync ccache base-devel binutils fakeroot tinyxml awk file inetutils > ${LIBS_LOG} 2>&1
      sudo pacman -S --noconfirm --needed $(grep depends PKGBUILD | cut -f2 -d=|tr -d \'\(\) | tr '\n' ' ') >> ${LIBS_LOG} 2>&1
      ;;
    *)
      echo "Unknown distribution base: ${LP3D_BASE}"
      exit 3
      ;;
  esac

  # Source update_config_files.sh"
  echo "Source update_config_files.sh..."
  export _EXPORT_CONFIG_ONLY_=1
  export _PRO_FILE_PWD_=${WD}/mainApp
  set +x && source builds/utilities/update-config-files.sh && set -x

  # List 'LP3D_*' environment variables
  set +x && echo && echo "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do echo $line=${!line};done && set -x

  # Make sure we have the distribution output paths
  [ ! -d "${LP3D_DIST_DIR_PATH}" ] && \
  mkdir -p "${LP3D_DIST_DIR_PATH}" || :

  # Download LDraw library archive files
  echo "Downloading archive libraries..."
  [ ! -f "${LP3D_3RD_PARTY_PATH}/lpub3dldrawunf.zip" ] && \
  wget -q https://library.ldraw.org/library/unofficial/ldrawunf.zip -O ${LP3D_3RD_PARTY_PATH}/lpub3dldrawunf.zip || :
  [ ! -f "${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip" ] && \
  ( cd ${LP3D_DIST_DIR_PATH} && ln -sf "${LP3D_3RD_PARTY_PATH}/lpub3dldrawunf.zip" lpub3dldrawunf.zip ) || \
  echo "${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip exists. Nothing to do."

  [ ! -f "${LP3D_3RD_PARTY_PATH}/complete.zip" ] && \
  wget -q https://library.ldraw.org/library/updates/complete.zip -O ${LP3D_3RD_PARTY_PATH}/complete.zip || :
  [ ! -f "${LP3D_DIST_DIR_PATH}/complete.zip" ] && \
  ( cd ${LP3D_DIST_DIR_PATH} && ln -sf "${LP3D_3RD_PARTY_PATH}/complete.zip" complete.zip ) || \
  echo "${LP3D_DIST_DIR_PATH}/complete.zip exists. Nothing to do."

  [ ! -f "${LP3D_3RD_PARTY_PATH}/tenteparts.zip" ] && \
  wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O ${LP3D_3RD_PARTY_PATH}/tenteparts.zip || :
  [ ! -f "${LP3D_DIST_DIR_PATH}/tenteparts.zip" ] && \
  ( cd ${LP3D_DIST_DIR_PATH} && ln -sf "${LP3D_3RD_PARTY_PATH}/tenteparts.zip" tenteparts.zip ) || \
  echo "${LP3D_DIST_DIR_PATH}/tenteparts.zip exists. Nothing to do."

  [ ! -f "${LP3D_3RD_PARTY_PATH}/vexiqparts.zip" ] && \
  wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip -O ${LP3D_3RD_PARTY_PATH}/vexiqparts.zip || :
  [ ! -f "${LP3D_DIST_DIR_PATH}/vexiqparts.zip" ] && \
  ( cd ${LP3D_DIST_DIR_PATH} && ln -sf "${LP3D_3RD_PARTY_PATH}/vexiqparts.zip" vexiqparts.zip ) || \
  echo "${LP3D_DIST_DIR_PATH}/vexiqparts.zip exists. Nothing to do."

  # Setup ldraw parts library directory
  if [ ! -d "${LP3D_LDRAW_DIR}/parts" ]; then
    [ ! -d "${LP3D_LDRAW_DIR}" ] && mkdir -p ${LP3D_LDRAW_DIR} || :
    ( cd ${LP3D_3RD_PARTY_PATH} && unzip -od ./ -q complete.zip )
    if [ -d "${LP3D_LDRAW_DIR}/parts" ]; then
      echo "LDraw library extracted to ${LP3D_LDRAW_DIR}"
    else
      echo "ERROR - LDraw library was not extracted."
    fi
  else
    echo "Directory ${LP3D_LDRAW_DIR} exists. Nothing to do."
  fi

  # Link $HOME/ldraw directory to shared ldraw path
  if [[ ! -L "${LDRAWDIR}" && ! -d "${LDRAWDIR}" ]]; then
    ( cd ~ && ln -sf "${LP3D_LDRAW_DIR}" ldraw ) && \
    echo "${LP3D_LDRAW_DIR} linked to ${LDRAWDIR}"
  else
    echo "Directory ${LDRAWDIR} exists. Nothing to do."
  fi

  # Build renderers LDGLite, LDView, and POV-Ray
  chmod a+x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
  if [ $? != 0 ]; then
    echo "ERROR - Create renderers failed with return code $?."
    exit 5
  fi
fi

# Compile LPub3D for analysis
if [ -n "${LP3D_ANALYZE}" ]; then
  echo "DEBUG LIST ${LP3D_DIST_DIR_PATH}..." && \
  ls -al ${LP3D_DIST_DIR_PATH}/ && echo "DEBUG END" && echo

  # Qmake setup
  if which qmake-qt5 >/dev/null 2>&1; then
    QMAKE_EXEC=qmake-qt5
  else
    QMAKE_EXEC=qmake
  fi
  ${QMAKE_EXEC} -v

  # Qmake configure
  if [[ "${LP3D_APPIMAGE}" == "false" ]]; then
    case ${LP3D_BASE} in
      "ubuntu")
        distropkg=deb ;;
      "fedora")
        distropkg=rpm ;;
      "archlinux")
        distropkg=pkg ;;
    esac
  else
    distropkg=api
  fi
  ${QMAKE_EXEC} -nocache QMAKE_STRIP=: CONFIG+=release CONFIG-=debug_and_release CONFIG+=${distropkg}

  # Compile command
  make -j$(nproc)
fi

exit 0
