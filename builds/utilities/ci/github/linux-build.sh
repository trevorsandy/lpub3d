#!/bin/bash
# Trevor SANDY
# Last Update May 22, 2022
#
# This script is called from .github/workflows/build.yml
#
# Options:
# BUILD=<ubuntu|fedora|archlinux>[-<amd64|amd32|arm64|arm32>-appimage]
# ARCH=<amd64|amd32|arm64|arm32>
# APPIMAGE=true|false
#
# Run command example:
# bash -ex env BUILD=ubuntu ARCH=amd64 APPIMAGE=1 builds/utilities/ci/github/linux-build.sh
#

if [[ "$BUILD" == "" ]]; then
    echo "This script is called from .github/workflows/build.yml"
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
    exit 2
fi

build_base="${BUILD%%-*}"
build_arch="${ARCH:-`uname -m`}"
build_appimage="${APPIMAGE:-false}"
IFS=- read LP3D_BASE LP3D_ARCH LP3D_APPIMAGE <<< $BUILD

if [ -z "${LP3D_ARCH}" ]; then
    export LP3D_ARCH=${build_arch}
else
    export LP3D_ARCH
fi

if [ -z "${LP3D_APPIMAGE}" ]; then
    LP3D_APPIMAGE=${build_appimage}
fi

case "${LP3D_ARCH}" in
    "amd64"|"x86_64")
        export LP3D_QEMU="false" ;;
    *)
        export LP3D_QEMU="true" ;;
esac

case "${LP3D_APPIMAGE}" in
    "true"|"false")
        export LP3D_APPIMAGE ;;
    "appimage")
        export LP3D_APPIMAGE="true" ;;
    *)
        export LP3D_APPIMAGE=${build_appimage} ;;
esac

# automatic logging
WRITE_LOG=${WRITE_LOG:-true}
if [ "${WRITE_LOG}" = "true" ]; then
    [ ! -d "${LP3D_BUILDPKG_PATH}" ] && mkdir -p ${LP3D_BUILDPKG_PATH} || :
    [ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$LP3D_BUILDPKG_PATH || :
    f="${0##*/}"; f="${f%.*}"; f="${f}-${build_base}-${build_arch}"
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

case "${LP3D_BASE}" in
    "ubuntu"|"fedora"|"archlinux")
        export BUILD_OPT="default"
        export BUILD="${LP3D_BASE}"
        # Check commit for version tag
        if [[ "${GITHUB_REF}" == "refs/tags/"* ]]; then
          publish=$(echo "${GITHUB_REF_NAME}" | perl -nle 'print "yes" if m{^(?!$)(?:v[0-9]+\.[0-9]+\.[0-9]+_?[^\W]*)?$} || print "no"')
        fi
        if [[ "${publish}" = "yes" || "${LP3D_COMMIT_MSG}" = *"BUILD_AMD"* ]]; then
          export LP3D_COMMIT_MSG="$(echo ${LP3D_COMMIT_MSG} BUILD_ALL | awk '{print toupper($0)}')"
        else
          export LP3D_COMMIT_MSG="$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')"
        fi
        if [[ "${LP3D_QEMU}" = "true" && "${LP3D_COMMIT_MSG}" = *"BUILD_AMD"* ]]; then
            echo "Skipping QEMU build, BUILD_AMD detected."
            exit 0;
        fi
        if [[ "${GITHUB_EVENT_NAME}" = "push" && ! "${LP3D_COMMIT_MSG}" = *"BUILD_ALL"* ]]; then
            if [ "${LP3D_QEMU}" = "false" ]; then
                [ "${LP3D_APPIMAGE}" != "true" ] && export BUILD_OPT="verify" || :
                source builds/utilities/ci/github/linux-multiarch-build.sh
            else
                echo "Unsupported option set QEMU: ${LP3D_QEMU}, GITHUB_EVENT: ${GITHUB_EVENT_NAME}, COMMIT_MSG: ${LP3D_COMMIT_MSG}"
                exit 0
            fi
        elif [[ "${LP3D_QEMU}" = "true" || "${LP3D_APPIMAGE}" = "true" ]]; then
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
