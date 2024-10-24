#!/bin/bash
# Trevor SANDY
# Last Update October 21, 2024

function ShowHelp() {
    echo
    echo "This script is called from .github/workflows/prod_ci_build.yml"
    echo
    echo "Run command:"
    echo "bash -ex builds/utilities/ci/github/$0"
    echo
    echo "Reference:"
    echo "Setting the follwing evnironment variables accordingly will trigger"
    echo "a full build versus a verify build which does not perform the package operation"
    echo
    echo "GITHUB_REF set to \"refs/tags/v2.4.8\" will trigger a full build"
    echo "GITHUB_REF_NAME use branch or tag \"v2.4.8\". A tag will trigger a full build"
    echo "LP3D_COMMIT_MSG keyword \"release_build\" will trigger a full build"
    echo
    echo "To replicate a 'GITHUB' build locally:"
    echo "Set the below variables accordingly"
    echo "then cut and paste in your console to run."
    echo
    echo "LP3D_BUILDPKG_PATH=/Users/trevorsandy/Development/buildpkg \\"
    echo "[ ! -d \"\$LP3D_BUILDPKG_PATH\" ] && mkdir -p \"\$LP3D_BUILDPKG_PATH\" || : \\"
    echo "chmod +x lpub3d/builds/utilities/ci/github/$0 && \\"
    echo "bash -ex env \\"
    echo "GITHUB=true \\"
    echo "GITHUB_REPOSITORY=trevorsandy/lpub3d \\"
    echo "GITHUB_REF=refs/heads/master \\"
    echo "GITHUB_REF_NAME=master \\"
    echo "GITHUB_EVENT_NAME=push \\"
    echo "GITHUB_WORKSPACE=/Users/trevorsandy/Development/lpub3d \\"
    echo "LP3D_COMMIT_MSG=\"LPub3D continuous development_build\" \\"
    echo "LP3D_3RD_PARTY_PATH=/Users/trevorsandy/Development \\"
    echo "LP3D_BUILDPKG_PATH=\${LP3D_BUILDPKG_PATH} \\"
    echo "./lpub3d/builds/utilities/ci/github/$0"
    echo
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -?|-h|--help) ShowHelp; exit 0 ;;
        *) echo "Unknown parameter passed: '$1'. Use -? to show help."; exit 1 ;;
    esac
    shift
done

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
oldIFS=$IFS; IFS='/' read -ra LP3D_SLUGS <<< "${GITHUB_REPOSITORY}"; IFS=$oldIFS;
export LPUB3D=${LP3D_SLUGS[1]}
export LP3D_ARCH=${LP3D_ARCH:-$(uname -m)}
export LP3D_LDRAW_DIR="${LP3D_3RD_PARTY_PATH}/ldraw"
export LDRAWDIR_ROOT=${LDRAWDIR_ROOT:-$HOME/Library}
export LDRAWDIR=${LDRAWDIR:-$LDRAWDIR_ROOT/LDraw}
export CI=${CI:-true}
export GITHUB=${GITHUB:-true}
export LP3D_CPU_CORES

# Check if build is on stale commit
oldIFS=$IFS; IFS='/' read -ra LP3D_SLUGS <<< "${GITHUB_REPOSITORY}"; IFS=$oldIFS;
export LP3D_PROJECT_NAME="${LP3D_SLUGS[1]}"
export LP3D_GREP=grep
[ ! -f "repo.txt" ] && \
curl -s -H "Authorization: Bearer ${GITHUB_TOKEN}" https://api.github.com/repos/${GITHUB_REPOSITORY}/commits/master -o repo.txt
export LP3D_REMOTE=$(${LP3D_GREP} -Po '(?<=: \")(([a-z0-9])\w+)(?=\")' -m 1 repo.txt)
export LP3D_LOCAL=$(git rev-parse HEAD)
if [[ "$LP3D_REMOTE" != "$LP3D_LOCAL" ]]; then
  echo "WARNING - Build no longer current. Rmote: '$LP3D_REMOTE', Local: '$LP3D_LOCAL' - aborting build."
  [ -f "repo.txt" ] && echo "Repo response:" && cat repo.txt || :
  exit 0
fi

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
  mkdir -p "$LP3D_LDRAW_DIR" && echo "Created LDraw library $LP3D_LDRAW_DIR"
else
  echo "Using cached LDraw library $LP3D_LDRAW_DIR"
fi

# Setup LDraw parts test path link
if [ ! -d "$LDRAWDIR" ]; then
  ln -sf "$LP3D_LDRAW_DIR" "LDRAWDIR" && \
  echo "$LP3D_LDRAW_DIR linked to $LDRAWDIR"
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
chmod a+x builds/macx/CreateDmg.sh && \
env BUILD_OPT="${BUILD_OPT}" \
CI="${CI}" \
GITHUB="${GITHUB}" \
LDRAWDIR_ROOT="${LDRAWDIR_ROOT}" \
LDRAWDIR="${LDRAWDIR}" \
LP3D_3RD_DIST_DIR="${LP3D_3RD_DIST_DIR}" \
LP3D_ARCH="${LP3D_ARCH}" \
LP3D_COMMIT_MSG="${LP3D_COMMIT_MSG}" \
LP3D_CPU_CORES="${LP3D_CPU_CORES}" \
LP3D_DIST_DIR_PATH="${LP3D_DIST_DIR_PATH}" \
LP3D_LDRAW_DIR="${LP3D_LDRAW_DIR}" \
LP3D_LOG_PATH="${LP3D_LOG_PATH}" \
LPUB3D="${LPUB3D}" \
WRITE_LOG="false" \
./builds/macx/CreateDmg.sh

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
