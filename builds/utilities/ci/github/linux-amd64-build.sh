#!/bin/bash
# Trevor SANDY
# Last Update August 06, 2021
#
# This script is called from builds/utilities/ci/github/build.sh
#
# Run command:
# bash -ex builds/utilities/ci/github/linux-amd64-build.sh

if [[ "$LP3D_BASE" == "" ]]; then
    echo "Usage: env LP3D_BASE=... bash $0"
    exit 2
fi

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
    # Elapsed execution time
    set +x
    ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
    echo "----------------------------------------------------"
    echo "${ME} for (${docker_base}-${docker_dist}-${docker_arch}) Finished!"
    echo "$ELAPSED"
    echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# resolve docker image options
docker_tag=
docker_dist=
docker_image=
docker_base="$LP3D_BASE"
docker_arch="amd64"
docker_platform="--platform linux/${docker_arch}"
case "${docker_base}" in
    "ubuntu")
        docker_dist="bionic"
        docker_tag="${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        ;;
    "fedora")
        docker_dist="33"
        docker_tag="${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        ;;
    "archlinux")
        docker_dist="latest"
        docker_tag="${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        ;;
    *)
        echo "Unknown build bas: ${docker_base}"
        exit 3
        ;;
esac

# format the log name - SOURCED if $1 is empty
WRITE_LOG=${WRITE_LOG:-true}
ME="linux-amd64-build"
[ "$(basename $0)" = "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# make sure we're in the repository root directory
cd ${GITHUB_WORKSPACE}

# prepare output directory
[ ! -d "${LP3D_BUILDPKG_PATH}" ] && \
mkdir -p ${LP3D_BUILDPKG_PATH} || :
out_path="${LP3D_BUILDPKG_PATH}"

# prepare third-party distribution directory
[ ! -d "${LP3D_3RD_PARTY_PATH}/${docker_base}_${docker_arch}" ] && \
mkdir -p ${LP3D_3RD_PARTY_PATH}/${docker_base}_${docker_arch} || :
base_path="${LP3D_3RD_PARTY_PATH}/${docker_base}_${docker_arch}"

# automatic logging
if [ "${WRITE_LOG}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"; f="${f}-${docker_base}-${docker_dist}-${docker_arch}"
    f="${out_path}/${f}"
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

# prepare ldraw directory
dist_path="${LP3D_3RD_PARTY_PATH}"
ldraw_path="${dist_path}/ldraw"
[ ! -f "${dist_path}/lpub3dldrawunf.zip" ] && \
wget -q http://www.ldraw.org/library/unofficial/ldrawunf.zip -O ${dist_path}/lpub3dldrawunf.zip || :
[ ! -f "${dist_path}/complete.zip" ] && \
wget -q http://www.ldraw.org/library/updates/complete.zip -O ${dist_path}/complete.zip || :
[ ! -f "${dist_path}/tenteparts.zip" ] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -O ${dist_path}/tenteparts.zip || :
[ ! -f "${dist_path}/vexiqparts.zip" ] && \
wget -q https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip -O ${dist_path}/vexiqparts.zip || :
if [ ! -d "${ldraw_path}/parts" ]; then
    [ ! -d "${ldraw_path}" ] && mkdir -p ${ldraw_path} || :
    (cd ${dist_path} && unzip -od ./ -q complete.zip)
    if [ -d "${ldraw_path}/parts" ]; then
        echo "LDraw library extracted."
    else
        echo "ERROR - LDraw library was not extracted."
    fi
fi

# Link LDraw libraries to base path
[ ! -f "${base_path}/lpub3dldrawunf.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/lpub3dldrawunf.zip lpub3dldrawunf.zip) || \
echo "${base_path}/lpub3dldrawunf.zip exists. Nothing to do."
[ ! -f "${base_path}/complete.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/complete.zip complete.zip) || \
echo "${base_path}/complete.zip exists. Nothing to do."
[ ! -f "${base_path}/tenteparts.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/tenteparts.zip tenteparts.zip) || \
echo "${base_path}/tenteparts.zip exists. Nothing to do."
[ ! -f "${base_path}/vexiqparts.zip" ] && \
(cd ${base_path} && ln -sf ${dist_path}/vexiqparts.zip vexiqparts.zip) || \
echo "${base_path}/vexiqparts.zip exists. Nothing to do."

# Trigger rebuild renderers if specified
ldglite_path=${base_path}/ldglite-1.3
ldview_path=${base_path}/ldview-4.4
povray_path=${base_path}/lpub3d_trace_cui-3.8
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_LDGLITE"* ]] && \
echo "'Build LDGLite' detected." && [ -d "${ldglite_path}" ] && \
rm -rf "${ldglite_path}" && echo "Cached ${ldglite_path} deleted" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_LDVIEW"* ]] && \
echo "'Build LDView' detected." && [ -d "${ldview_path}" ] && \
rm -rf "${ldview_path}" && echo "Cached ${ldview_path} deleted" || :
[[ "${LP3D_COMMIT_MSG}" == *"BUILD_POVRAY"* ]] && \
echo "'Build POV-Ray' detected." && [ -d "${povray_path}" ] && \
rm -rf "${povray_path}" && echo "Cached ${povray_path} deleted" || :

# run builds with privileged user account required to load dependent
gid="$(id -g)"
uid="$(id -u)"
name="$(id -un)"

# generate Dockerfile and run CMD command script
cat << pbEOF >${out_path}/Dockerfile
FROM ${docker_tag}
pbEOF
case "${docker_base}" in
    "ubuntu")
        cp -f builds/linux/obs/alldeps/debian/control .
        cp -f builds/linux/CreateDeb.sh .
cat << pbEOF >>${out_path}/Dockerfile
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y apt-utils git wget unzip lintian build-essential debhelper fakeroot ccache lsb-release
RUN apt-get install -y autotools-dev autoconf pkg-config libtool curl zip
RUN apt-get install -y xvfb desktop-file-utils
RUN apt-get install -y $(grep Build-Depends control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,)
RUN apt-get install -y sudo \\
    && groupadd -r ${name} -g ${gid} \\
    && useradd -u ${uid} -r -g ${name} -m -d /${name} -s /sbin/nologin -c "Build pkg user" ${name} \\
    && chmod 755 /${name} \\
    && echo "${name} ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${name} \\
    && chmod 0440 /etc/sudoers.d/${name}
WORKDIR /${name}
USER ${name}
VOLUME ["/in", "/buildpkg", "/dist", "/${name}/ldraw"]
ADD --chown=${name}:${name} docker-run-CMD.sh /${name}
ADD --chown=${name}:${name} CreateDeb.sh /${name}
CMD /bin/bash
pbEOF
cat << pbEOF >${out_path}/docker-run-CMD.sh
#!/bin/bash
cd ~/ \\
&& mkdir -p ./debbuild/SOURCES \\
&& (cd debbuild && ln -sf /dist/${docker_base}_${docker_arch}/ lpub3d_linux_3rdparty) \\
&& (cd debbuild/SOURCES && cp -af /dist/*.zip .) \\
&& sudo chown -R ${name}:${name} ./debbuild/.* \\
&& chmod a+x CreateDeb.sh && ./CreateDeb.sh \\
&& if test -d /buildpkg; then \\
  cd ~/; \\
  ls -al ./; \\
  ls -al ./debbuild/; \\
  ls -al ./debbuild/lpub3d_linux_3rdparty/ 2>/dev/null || :; \\
  sudo mv -f ./debbuild/*.deb* /buildpkg/ 2>/dev/null || :; \\
  sudo mv -f ./debbuild/*.xz /buildpkg/ 2>/dev/null || :; \\
  sudo mv -f ./debbuild/*.buildinfo /buildpkg/ 2>/dev/null || :; \\
  sudo mv -f ./debbuild/*.dsc /buildpkg/ 2>/dev/null || :; \\
  sudo mv -f ./debbuild/*.changes /buildpkg/ 2>/dev/null || :; \\
  sudo mv -f ./debbuild/*log /buildpkg/ 2>/dev/null || :; \\
  sudo mv -f ./*log /buildpkg/ 2>/dev/null || :; \\
  ls -al /buildpkg/; \\
pbEOF
        ;;
    "fedora")
        cp -f builds/linux/obs/alldeps/lpub3d.spec .
        cp -f builds/linux/obs/lpub3d-rpmlintrc .
        cp -f builds/linux/CreateRpm.sh .
        sed -e '/Icon: lpub3d.xpm/d' \
            -e 's/<B_CNT>/1/' -i lpub3d.spec
cat << pbEOF >>${out_path}/Dockerfile
ADD lpub3d.spec /
RUN dnf install -y git wget unzip which rpmlint ccache dnf-plugins-core rpm-build
RUN dnf install -y xorg-x11-server-Xvfb desktop-file-utils
RUN dnf builddep -y /lpub3d.spec
RUN dnf install -y sudo \\
    && groupadd -r ${name} -g ${gid} \\
    && useradd -u ${uid} -r -g ${name} -m -d /${name} -s /sbin/nologin -c "Build pkg user" ${name} \\
    && chmod 755 /${name} \\
    && echo "${name} ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${name} \\
    && chmod 0440 /etc/sudoers.d/${name}
WORKDIR /${name}
USER ${name}
VOLUME ["/in", "/buildpkg", "/dist", "/${name}/ldraw"]
ADD --chown=${name}:${name} docker-run-CMD.sh /${name}
ADD --chown=${name}:${name} CreateRpm.sh /${name}
ADD lpub3d-rpmlintrc /etc/rpmlint
CMD /bin/bash
pbEOF
cat << pbEOF >${out_path}/docker-run-CMD.sh
#!/bin/bash
cd ~/ \\
&& mkdir -p ./rpmbuild/{BUILD,SOURCES} \\
&& (cd rpmbuild/BUILD && ln -sf /dist/${docker_base}_${docker_arch}/ lpub3d_linux_3rdparty) \\
&& (cd rpmbuild/SOURCES && cp -af /dist/*.zip .) \\
&& sudo chown -R ${name}:${name} ./rpmbuild/.* \\
&& sudo ln -s //lib64/libXext.so.6.4.0 /usr/lib/libXext.so 2>/dev/null || : \\
&& chmod a+x CreateRpm.sh && ./CreateRpm.sh \\
&& if test -d /buildpkg; then \\
    cd ~/; \\
    ls -al ./; \\
    ls -al ./rpmbuild/; \\
    ls -al ./rpmbuild/RPMS/; \\
    ls -al ./rpmbuild/BUILD/; \\
    ls -al ./rpmbuild/BUILD/lpub3d_linux_3rdparty/; \\
    sudo mv -f ./rpmbuild/RPMS/$(uname -m)/*.rpm* /buildpkg/ 2>/dev/null || :; \\
    sudo mv -f ./rpmbuild/BUILD/*.log /buildpkg/ 2>/dev/null || :; \\
    sudo mv -f ./*log /buildpkg/ 2>/dev/null || :; \\
    ls -al /buildpkg; \\
pbEOF
        ;;
    "archlinux")
        cp -f builds/linux/obs/alldeps/PKGBUILD .
        cp -f builds/linux/CreatePkg.sh .
cat << pbEOF >>${out_path}/Dockerfile
RUN pacman -Suy --noconfirm
# WORKAROUND for glibc 2.33 and old Docker
# See https://github.com/actions/virtual-environments/issues/2658
# Thanks to https://github.com/lxqt/lxqt-panel/pull/1562
RUN patched_glibc=glibc-linux4-2.33-4-x86_64.pkg.tar.zst && \\
    curl -LO "https://repo.archlinuxcn.org/x86_64/\$patched_glibc" && \\
    bsdtar -C / -xvf "\$patched_glibc"
RUN pacman -Sy --noconfirm git wget unzip ccache base-devel binutils fakeroot tinyxml awk file inetutils
RUN pacman -S --noconfirm --needed xorg-server-xvfb desktop-file-utils
RUN pacman -S --noconfirm --needed $(grep depends PKGBUILD | cut -f2 -d=|tr -d \'\(\) | tr '\n' ' ')
RUN pacman -S --noconfirm --needed sudo \\
    && groupadd -r ${name} -g ${gid} \\
    && useradd -u ${uid} -r -g ${name} -m -d /${name} -s /sbin/nologin -c "Build pkg user" ${name} \\
    && chmod 755 /${name} \\
    && echo "${name} ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${name} \\
    && chmod 0440 /etc/sudoers.d/${name}
WORKDIR /${name}
USER ${name}
VOLUME ["/in", "/buildpkg", "/dist", "/${name}/ldraw"]
ADD --chown=${name}:${name} docker-run-CMD.sh /${name}
ADD --chown=${name}:${name} CreatePkg.sh /${name}
CMD /bin/bash
pbEOF
cat << pbEOF >${out_path}/docker-run-CMD.sh
#!/bin/bash
cd ~/ \\
&& mkdir -p ./pkgbuild/src \\
&& (cd pkgbuild/src && ln -sf /dist/${docker_base}_${docker_arch}/ lpub3d_linux_3rdparty) \\
&& (cd pkgbuild && cp -af /dist/*.zip .) \\
&& sudo chown -R ${name}:${name} ./pkgbuild/.* \\
&& chmod a+x CreatePkg.sh && ./CreatePkg.sh \\
&& if test -d /buildpkg; then \\
    cd ~/; \\
    ls -al ./; \\
    ls -al ./pkgbuild/; \\
    ls -al ./pkgbuild/src/; \\
    ls -al ./pkgbuild/src/lpub3d_linux_3rdparty/; \\
    sudo mv -f ./pkgbuild/*.zst* /buildpkg/ 2>/dev/null || :; \\
    sudo mv -f ./pkgbuild/src/*.log /buildpkg/ 2>/dev/null || :; \\
    sudo mv -f ./*log /buildpkg/ 2>/dev/null || :; \\
    ls -al /buildpkg/; \\
pbEOF
        ;;
    *)
    echo "ERROR - Unknown distribution base: ${docker_base}_${docker_arch}"
    exit 3
        ;;
esac
cat << pbEOF >>${out_path}/docker-run-CMD.sh
  sudo cp -af ./*.log /buildpkg/ 2>/dev/null || :; \\
fi
pbEOF

# add Dockerfile to context
cp -f ${out_path}/Dockerfile .

# add run CMD script to context and set executable
cp -f ${out_path}/docker-run-CMD.sh . && chmod a+x docker-run-CMD.sh

# Docker Hub login
echo "Login to Docker Hub..."
echo ${DOCKER_HUB_TOKEN} | docker login --username ${DOCKER_USERNAME} --password-stdin

# reporitory
IFS='/' read -ra LP3D_SLUGS <<< "${GITHUB_REPOSITORY}"; unset IFS;
LPUB3D=${LP3D_SLUGS[1]}

# build docker image
docker_build_opts=(${docker_platform})
if [ "${DOCKER_CACHE}" = "false" ]; then
    docker_build_opts+=(
        --pull
        --no-cache
    )
fi
docker build --rm \
    "${docker_build_opts[@]}" \
    -f Dockerfile \
    -t ${docker_image} .

# marshall docker run options
common_docker_opts+=(
    -e CI="${CI}"
    -e GITHUB="${GITHUB}"
    -e LPUB3D="${LPUB3D}"
    -e DOCKER="true"
    -v "${PWD}":/in
    -v "${out_path}":/buildpkg
    -v "${dist_path}":/dist
    -v "${ldraw_path}":/${name}/ldraw
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
    /bin/bash -ex "./docker-run-CMD.sh"

test -d ${out_path} && ls -al ${out_path} || :

exit 0
