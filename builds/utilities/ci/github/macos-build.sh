#!/bin/bash
# Trevor SANDY
# Last Update September 22, 2024
#
# This script is called from .github/workflows/build.yml
#
# Run command:
# bash -ex builds/utilities/ci/github/macos-build.sh
#
# LOCAL 'GITHUB' RUN - set parameters accordingly then cut and paste in console to run.
: <<'BLOCK_COMMENT'
export GITHUB=true
export GITHUB_REPOSITORY=trevorsandy/lpub3d
export GITHUB_REF=refs/heads/master # "refs/tags/v2.4.8" will BUILD_ALL
export GITHUB_REF_NAME=master       # branch or Tag "v2.4.8" - Tag will trigger BUILD_ALL
export GITHUB_EVENT_NAME=push
export GITHUB_WORKSPACE=/Users/trevorsandy/Development/lpub3d
export LP3D_COMMIT_MSG="LPub3D continuous development_build"     # release_build will trigger BUILD_ALL
export LP3D_3RD_PARTY_PATH=/Users/trevorsandy/Development
export LP3D_BUILDPKG_PATH=/Users/trevorsandy/Development/buildpkg
[ ! -d "$LP3D_BUILDPKG_PATH" ] && mkdir -p "$LP3D_BUILDPKG_PATH" || :
chmod +x lpub3d/builds/utilities/ci/github/macos-build.sh && \
       ./lpub3d/builds/utilities/ci/github/macos-build.sh

BLOCK_COMMENT

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  ME="${ME} for (${LP3D_ARCH})"
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "$ME Verification Finished!"
  else
    echo "$ME Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

brew_install() {
  # Usage: 
  # brew_install qt@5
  if brew list $1 &>/dev/null; then
    echo "${1} is already installed."
  else
    case $1 in
    xquartz)
      brew install --cask $1
    ;;
    qt@5)
      brew install $1
      echo "Force linking to 'keg-only' instance of Qt..."
      brew link --force $1
    ;;
    *)
      brew install $1
    ;;
    esac
  fi
}

# Grab the commit message
if  [ "${LP3D_COMMIT_MSG}" = *"QUICK_BUILD"* ]; then
  echo "NOTICE - Quick build detected, $0 will not continue."
  exit 0
fi

# Grab the script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
# automatic logging
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_BUILDPKG_PATH}" ] && LP3D_BUILDPKG_PATH=$(cd ../ && echo $PWD/buildpkg)
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$LP3D_BUILDPKG_PATH || :
[ ! -d "${LP3D_LOG_PATH}" ] && mkdir -p ${LP3D_LOG_PATH} || :
if [ "${WRITE_LOG}" = "true" ]; then
    f="${LP3D_LOG_PATH}/$ME"
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
fi

export WRITE_LOG
export LP3D_LOG_PATH

# make sure we're in the repository root directory
cd "${GITHUB_WORKSPACE}"

# Make sure we have the distribution output paths
export LP3D_DIST_DIR_PATH="${LP3D_3RD_PARTY_PATH}/macos"
[ ! -d "${LP3D_DIST_DIR_PATH}" ] && \
mkdir -p "${LP3D_DIST_DIR_PATH}" || :

# Prepare output directory
LP3D_OUT_PATH="${LP3D_BUILDPKG_PATH}"
[ ! -d "${LP3D_OUT_PATH}" ] && \
mkdir -p ${LP3D_OUT_PATH} || :

# Set variables
IFS='/' read -ra LP3D_SLUGS <<< "${GITHUB_REPOSITORY}"; unset IFS;
export LPUB3D=${SLUG_PARTS[1]}
export LP3D_ARCH=${LP3D_ARCH:-$(uname -m)}
export LP3D_LDRAW_DIR="${LP3D_3RD_PARTY_PATH}/ldraw"
export LDRAWDIR_ROOT=${LDRAWDIR_ROOT:-$HOME/}
export LDRAWDIR=${LDRAWDIR:-$HOME/LDraw}
export CI=${CI:-true}
export GITHUB=${GITHUB:-true}
export LP3D_CPU_CORES

# Check commit for version tag
if [[ "${GITHUB_REF}" == "refs/tags/"* ]] ; then
  publish=$(echo "${GITHUB_REF_NAME}" | perl -nle 'print "yes" if m{^(?!$)(?:v[0-9]+\.[0-9]+\.[0-9]+_?[^\W]*)?$} || print "no"')  #'
fi
export LP3D_COMMIT_MSG="$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')"
if [[ "${publish}" == "yes" || "${LP3D_COMMIT_MSG}" =~ (RELEASE_BUILD) ]]; then
  export LP3D_COMMIT_MSG="${LP3D_COMMIT_MSG} BUILD_ALL"
fi
if [[ "${GITHUB_EVENT_NAME}" == "push" && ! "${LP3D_COMMIT_MSG}" == *"BUILD_ALL"* ]]; then
  export BUILD_OPT="verify"
fi

# Setup ldraw parts library directory
if [ ! -d "$LP3D_LDRAW_DIR" ]; then
  mkdir -p "$LP3D_LDRAW_DIR"
  if [ -d "$LP3D_LDRAW_DIR" ]; then
    echo "Created LDraw library $LP3D_LDRAW_DIR"
  fi
else
  echo "Using cached LDraw library $LP3D_LDRAW_DIR"
fi
if [ ! -d "$HOME/LDraw" ]; then
  ln -sf "$LP3D_LDRAW_DIR" "$HOME/LDraw" && \
  echo "$LP3D_LDRAW_DIR linked to $HOME/LDraw"
fi

# Make sure Qt is properly setup
echo "Install 'cask' instance of XQuartz..." && brew_install xquartz
echo "Install 'keg-only' instance of Qt..." && brew_install qt@5
echo "Install coreutils..." && brew_install coreutils
echo "Install wget..." && brew_install wget

# Link gsha512sum to sha512sum
if [ ! -f "/usr/local/bin/sha512sum" ]; then
  sudo ln -sf "/usr/local/bin/gsha512sum"  "/usr/local/bin/sha512sum" && \
  echo "/usr/local/bin/gsha512sum linked to /usr/local/bin/sha512sum"
fi

# Create dmgbuild working directory
[ ! -d "../dmgbuild" ] && mkdir -p ../dmgbuild;
DmgBuildPath=$(cd ../ && echo "$PWD/dmgbuild")

# Create LP3D_DIST_DIR_PATH link
export LP3D_3RD_DIST_DIR=lpub3d_macos_3rdparty
if [ ! -d "${DmgBuildPath}/${LP3D_3RD_DIST_DIR}" ]; then
  ln -sf "$LP3D_DIST_DIR_PATH" "${DmgBuildPath}/${LP3D_3RD_DIST_DIR}" && \
  if test -d "${DmgBuildPath}/${LP3D_3RD_DIST_DIR}"; then
    echo "$LP3D_DIST_DIR_PATH linked to ${DmgBuildPath}/${LP3D_3RD_DIST_DIR}"
  fi
else
  echo "Using cached 3rd Party repository ${DmgBuildPath}/${LP3D_3RD_DIST_DIR}"
fi

# Download LDraw library archive files
echo "Downloading archive libraries..."
[[ "${GITHUB}" = "true" && ! -f "${LP3D_3RD_PARTY_PATH}/lpub3dldrawunf.zip" ]] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/lpub3dldrawunf.zip -O ${LP3D_3RD_PARTY_PATH}/lpub3dldrawunf.zip || :
[ ! -f "${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip" ] && \
ln -sf "${LP3D_3RD_PARTY_PATH}/lpub3dldrawunf.zip" "${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip" || :
cp -f ${LP3D_3RD_PARTY_PATH}/lpub3dldrawunf.zip mainApp/extras
[ ! -f "${LP3D_3RD_PARTY_PATH}/complete.zip" ] && \
wget -q https://library.ldraw.org/library/updates/complete.zip -O ${LP3D_3RD_PARTY_PATH}/complete.zip || :
[ ! -f "${LP3D_DIST_DIR_PATH}/complete.zip" ] && \
ln -sf "${LP3D_3RD_PARTY_PATH}/complete.zip" "${LP3D_DIST_DIR_PATH}/complete.zip" || :
cp -f ${LP3D_3RD_PARTY_PATH}/complete.zip mainApp/extras
[ ! -f "${LP3D_3RD_PARTY_PATH}/tenteparts.zip" ] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O ${LP3D_3RD_PARTY_PATH}/tenteparts.zip || :
[ ! -f "${LP3D_DIST_DIR_PATH}/tenteparts.zip" ] && \
ln -sf "${LP3D_3RD_PARTY_PATH}/tenteparts.zip" "${LP3D_DIST_DIR_PATH}/tenteparts.zip" || :
cp -f ${LP3D_3RD_PARTY_PATH}/tenteparts.zip mainApp/extras
[ ! -f "${LP3D_3RD_PARTY_PATH}/vexiqparts.zip" ] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip -O ${LP3D_3RD_PARTY_PATH}/vexiqparts.zip || :
[ ! -f "${LP3D_DIST_DIR_PATH}/vexiqparts.zip" ] && \
ln -sf "${LP3D_3RD_PARTY_PATH}/vexiqparts.zip" "${LP3D_DIST_DIR_PATH}/vexiqparts.zip" || :
cp -f ${LP3D_3RD_PARTY_PATH}/vexiqparts.zip mainApp/extras

# Trigger rebuild renderers if specified
ldglite_path=${LP3D_DIST_DIR_PATH}/ldglite-1.3
ldview_path=${LP3D_DIST_DIR_PATH}/ldview-4.5
povray_path=${LP3D_DIST_DIR_PATH}/lpub3d_trace_cui-3.8
[[ "${LP3D_COMMIT_MSG}" == *"ALL_RENDERERS"* ]] && \
echo "'Build LDGLite, LDView and POV-Ray' detected." && \
LP3D_COMMIT_MSG="${LP3D_COMMIT_MSG} BUILD_LDGLITE BUILD_LDVIEW BUILD_POVRAY" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_LDGLITE"* ]] && \
echo "'Build LDGLite' detected." && [ -d "${ldglite_path}" ] && \
rm -rf "${ldglite_path}" && echo "Cached ${ldglite_path} deleted" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_LDVIEW"* ]] && \
echo "'Build LDView' detected." && [ -d "${ldview_path}" ] && \
rm -rf "${ldview_path}" && echo "Cached ${ldview_path} deleted" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_POVRAY"* ]] && \
echo "'Build POV-Ray' detected." && [ -d "${povray_path}" ] && \
rm -rf "${povray_path}" && echo "Cached ${povray_path} deleted" || :

# List 'LP3D_*' environment variables
echo && echo "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do echo $line=${!line};done

# Build dmg file
source builds/macx/CreateDmg.sh;

# Create hash file
[ -d "$DmgBuildPath/DMGS" ] && \
(
  cd $DmgBuildPath/DMGS && \
  file_DPkg=$(find . -name LPub3D-*.dmg -type f);
  [ -n "$file_DPkg" ] && sha512sum "${file_DPkg}" > "${file_DPkg}.sha512" || echo "ERROR - Failed to create hash file ${file_DPkg}.sha512"
) && \
echo "Package Files:" && find $DmgBuildPath/DMGS -type f

# Move AppImage build content to output
echo "Moving build assets and logs to output folder..."
cd ~/
mv -f ${DmgBuildPath}/DMGS/* ${LP3D_OUT_PATH}/ 2>/dev/null || :
mv -f ${GITHUB_WORKSPACE}/*.log ${LP3D_OUT_PATH} 2>/dev/null || :
mv -f ${DmgBuildPath}/*.log ${LP3D_OUT_PATH} 2>/dev/null || :
mv -f ./*.log ${LP3D_OUT_PATH} 2>/dev/null || :
mv -f ./*_assets.tar.gz ${LP3D_OUT_PATH} 2>/dev/null || :

exit 0
