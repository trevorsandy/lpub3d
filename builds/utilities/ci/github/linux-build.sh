#!/bin/bash
# Trevor SANDY
# Last Update October 18, 2024

function ShowHelp() {
    echo
    echo "This script is called from .github/workflows/prod_ci_build.yml"
    echo
    echo "Options:"
    echo "BUILD=<ubuntu|fedora|archlinux>[-<amd64|amd32|arm64|arm32>-appimage]"
    echo "ARCH=<amd64|amd32|arm64|arm32>"
    echo "APPIMAGE=true|false"
    echo
    echo "Run command examples:"
    echo "env BUILD=ubuntu bash -ex $0"
    echo "env BUILD=ubuntu-amd64 bash -ex $0"
    echo "env BUILD=ubuntu-amd64-appimage bash -ex $0"
    echo "env BUILD=ubuntu ARCH=amd64 APPIMAGE=1 bash -ex $0"
    echo
    echo "Run command example:"
    echo "bash -ex env BUILD=ubuntu ARCH=amd64 APPIMAGE=1 builds/utilities/ci/github/$0"
    echo
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -?|-h|--help) ShowHelp; exit 0 ;;
        *) echo "Unknown parameter passed: '$1'. Use -? to show help."; exit 1 ;;
    esac
    shift
done

build_base="${BUILD:-$(. /etc/os-release 2>/dev/null && echo $ID)}"
build_arch="${ARCH:-`uname -m`}"
build_appimage="${APPIMAGE:-false}"

oldIFS=$IFS && IFS=- read LP3D_BASE LP3D_ARCH LP3D_APPIMAGE <<< $BUILD && IFS=$oldIFS

[ -z "${LP3D_BASE}" ] && export LP3D_BASE=${build_base} || export LP3D_BASE
[ -z "${LP3D_ARCH}" ] && export LP3D_ARCH=${build_arch} || export LP3D_ARCH
[ -z "${LP3D_APPIMAGE}" ] && LP3D_APPIMAGE=${build_appimage} || :

case "${LP3D_ARCH}" in
    amd64|x86_64)
        export LP3D_QEMU="false" ;;
    *)
        export LP3D_QEMU="true" ;;
esac

case "${LP3D_APPIMAGE}" in
    true|false)
        export LP3D_APPIMAGE ;;
    appimage)
        export LP3D_APPIMAGE="true" ;;
    *)
        export LP3D_APPIMAGE=${build_appimage} ;;
esac

# automatic logging
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_BUILDPKG_PATH}" ] && LP3D_BUILDPKG_PATH=$(cd ../ && echo $PWD/buildpkg)
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$LP3D_BUILDPKG_PATH || :
[ ! -d "${LP3D_LOG_PATH}" ] && mkdir -p ${LP3D_LOG_PATH} || :
if [ "${WRITE_LOG}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"; f="${f}-${LP3D_BASE}-${LP3D_ARCH}"
    [ "${LP3D_APPIMAGE}" = "true" ] && f="${f}-appimage"
    f="${LP3D_LOG_PATH}/${f}"
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

# check if build is on stale commit
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

case "${LP3D_BASE}" in
    "ubuntu"|"fedora"|"archlinux")
        export CI=${CI:-true}
        export GITHUB=${GITHUB:-true}
        export BUILD_OPT="default"
        export BUILD="${LP3D_BASE}"
        export WRITE_LOG
        export LP3D_LOG_PATH
        export LP3D_COMMIT_MSG="$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')"
        # Check commit for version tag
        if [[ "${GITHUB_REF}" == "refs/tags/"* ]]; then
            publish=$(echo "${GITHUB_REF_NAME}" | perl -nle 'print "yes" if m{^(?!$)(?:v[0-9]+\.[0-9]+\.[0-9]+_?[^\W]*)?$} || print "no"')  #'
        fi
        if [[ "${publish}" == "yes" || "${LP3D_COMMIT_MSG}" =~ (RELEASE_BUILD) ]]; then
            export LP3D_COMMIT_MSG="$(echo ${LP3D_COMMIT_MSG} BUILD_ALL)"
        fi
        if [[ ! "${LP3D_COMMIT_MSG}" == *"BUILD_ALL"* ]]; then
            export BUILD_OPT="verify"
        fi
        if [[ "${LP3D_QEMU}" == "true" && (! "${LP3D_COMMIT_MSG}" =~ (BUILD_ALL) || "${LP3D_COMMIT_MSG}" =~ (SKIP_QEMU)) ]]; then
            echo "Skipping QEMU ${LP3D_ARCH} build."
            exit 0;
        fi
        if [[ "${LP3D_QEMU}" == "true" || "${LP3D_APPIMAGE}" == "true" ]]; then
            source builds/utilities/ci/github/linux-multiarch-build.sh
        else
            source builds/utilities/ci/github/linux-amd64-build.sh
        fi
        ;;
    *)
        echo "Unsupported build: ${LP3D_BASE}"
        exit 3
        ;;
esac

exit 0
