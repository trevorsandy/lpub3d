#!/bin/bash
# Trevor SANDY
# Last Update June 03, 2021
#
# This script is called from .github/workflows/build.yml
#
# Run command:
# bash -ex builds/utilities/ci/github/multiarch-build.sh

set -e
set -x

if [[ "$ARCH" == "" ]]; then
    echo "Usage: env ARCH=... bash $0"
    exit 2
fi

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "${0##*/} Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# resolve docker image options
docker_arch=
docker_base=
docker_tag=
docker_image=
docker_platform=
case "$ARCH" in
    "x86_64")
        export ARCH="x86_64"
        docker_base="ubuntu"
        docker_dist="focal"
        docker_tag="amd64/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch:-$ARCH}"
        docker_platform="--platform linux/amd64"
        ;;
    "i386"|"i686")
        export ARCH="i686"
        docker_arch="i386"
        docker_base="ubuntu"
        docker_dist="bionic"
        docker_tag="i386/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch:-$ARCH}"
        ;;
    "aarch64")
        docker_base="ubuntu"
        docker_dist="focal"
        docker_tag="arm64v8/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch:-$ARCH}"
        docker_platform="--platform linux/arm64"
        ;;
    "armhf")
        docker_base="ubuntu"
        docker_dist="xenial"
        docker_tag="arm32v7/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch:-$ARCH}"
        docker_platform="--platform linux/arm/v7"
        ;;
    *)
        echo "Unknown architecture: $ARCH"
        exit 3
        ;;
esac

# make sure we're in the repository root directory
repo_root="${GITHUB_WORKSPACE}"

# third-party distribution directory path
dist_path="$(readlink -f ${repo_root}/../third_party)"

# prepare output directory
mkdir -p ${repo_root}/../appimage_out
out_path="$(readlink -f ${repo_root}/../appimage_out/)"

if [ "${WRITE_LOG}" = "true" ]; then
    # automatic logging
    f="${out_path}/${0##*/}"; f="${f%.*}_${docker_dist}-${ARCH}.sh"
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

# set build-binaries-and-appimage.sh executable
chmod a+x builds/utilities/ci/github/build-binaries-and-appimage.sh

# run builds with privileged user account required to load dependent
gid="$(id -g)"
uid="$(id -u)"
name="$(id -un)"

# generate Dockerfile and build image
cat << pbEOF >${out_path}/Dockerfile
FROM ${docker_tag}
pbEOF
case "${docker_base}" in
    "debian"|"ubuntu")
cat << pbEOF >>${out_path}/Dockerfile
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y apt-utils git wget unzip lintian build-essential debhelper fakeroot ccache lsb-release
RUN apt-get install -y autotools-dev autoconf pkg-config libtool curl zip
RUN apt-get install -y xvfb desktop-file-utils
RUN apt-get install -y $(grep Build-Depends builds/linux/obs/alldeps/debian/control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,)
pbEOF
        ;;
    "fedora")
cat << pbEOF >>${out_path}/Dockerfile
RUN dnf install -y git wget unzip which rpmlint ccache dnf-plugins-core rpm-build
RUN dnf install -y xorg-x11-server-Xvfb desktop-file-utils
RUN dnf builddep -y builds/linux/obs/alldeps/lpub3d.spec
pbEOF
        ;;
    "centos")
cat << pbEOF >>${out_path}/Dockerfile
RUN yum install -y git wget unzip which rpmlint ccache dnf-plugins-core rpm-build yum-utils
RUN yum install -y xorg-x11-server-Xvfb desktop-file-utils
RUN yum builddep -y builds/linux/obs/alldeps/lpub3d.spec
RUN yum install -y sudo && yum clean all \\
pbEOF
        ;;
    "arch")
cat << pbEOF >>${out_path}/Dockerfile
RUN pacman -Suy --noconfirm
RUN pacman -Sy --noconfirm git sudo binutils fakeroot tinyxml awk file inetutils
RUN pacman -S --noconfirm $(grep depends builds/linux/obs/alldeps/PKGBUILD | cut -f2 -d=|tr -d \'\(\))
pbEOF
        ;;
    *)
    echo "Unknown distribution base: ${docker_base}"
    exit 3
        ;;
esac
cat << pbEOF >>${out_path}/Dockerfile
RUN groupadd -r ${name} -g ${gid} \\
    && useradd -u ${uid} -r -g ${name} -m -d /${name} -s /sbin/nologin -c "Build pkg user" ${name} \\
    && chmod 755 /${name}
WORKDIR /${name}
USER ${name}
VOLUME ["/ws","/out","/dist"]
CMD /bin/bash
pbEOF
docker build --rm \
     ${docker_platform} \
     -f ${out_path}/Dockerfile \
     -t ${docker_image} .

# marshall docker run options
common_docker_opts=(
    -e CI="${CI}"
    -e TERM="${TERM}"
    -e ARCH="${ARCH}"
    -e GITHUB="${GITHUB}"
    -e BUILD_CPUs="${BUILD_CPUs}"
    -e NO_CLEANUP="${NO_CLEANUP}"
    -e GITHUB_SHA="${GITHUB_SHA}"
    -e GITHUB_REF="${GITHUB_REF}"
    -i
    -v "${repo_root}":/ws
    -v "${dist_path}":/dist
    -v "${out_path}":/out
)

# make ctrl-c work
if [[ "$CI" != "true" ]] && [[ "$TERM" != "" ]]; then
    common_docker_opts+=("-t")
fi

# enforce using the same UID in the container as outside, so that the created files are owned by the caller
docker run \
    ${docker_platform} \
    --rm \
    --user "${uid}" \
    --privileged \
    --security-opt seccomp=unconfined \
    "${common_docker_opts[@]}" \
    "${docker_image}" \
    /bin/bash -xc "cp -a /ws/* . && ./builds/utilities/ci/github/build-binaries-and-appimage.sh"

# list the built files
ls -al ${out_path}/

Exit 0
