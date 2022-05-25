#!/bin/bash
# Trevor SANDY
# Last Update July 09, 2021
#
# This script is called from .github/workflows/build.yml
#
# Run command:
# bash -ex builds/utilities/ci/github/linux-miltiarch-build.sh

if [[ "$LP3D_ARCH" == "" ]]; then
    echo "Usage: env LP3D_ARCH=... bash $0"
    exit 2
fi

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  ME="${ME} for (${docker_base}-${docker_dist}-${docker_arch})"
  [ "${LP3D_APPIMAGE}" = "true" ] && \
  ME="${ME} for (${docker_base}-${docker_dist}-${docker_arch}-appimage)" || :
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "$ME Verification Finished!"
  else
    echo "$ME Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# resolve docker image options
docker_tag=
docker_image=
docker_dist="bionic"
docker_arch="${LP3D_ARCH:-amd64}"
docker_base="${LP3D_BASE:-ubuntu}"
docker_qemu="${LP3D_QEMU:-false}"
docker_platform="--platform linux/${LP3D_ARCH}"
case "${docker_base}" in
    "ubuntu")
        docker_dist="bionic" ;;
    "fedora")
        docker_dist="33" ;;
    "archlinux")
        docker_dist="latest" ;;
    *)
        echo "Unknown build base: ${docker_base}"
        exit 3
        ;;
esac
case "$LP3D_ARCH" in
    "amd64"|"x86_64")
        export LP3D_ARCH="amd64"
        ait_arch="x86_64"
        docker_tag="amd64/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        docker_platform="--platform linux/amd64"
        ;;
    "amd32"|"i386"|"i686")
        export LP3D_ARCH="i686"
        docker_arch="i386"
        ait_arch="${LP3D_ARCH}"
        docker_tag="i386/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        ;;
    "arm64"|"aarch64")
        ait_arch="aarch64"
        docker_tag="arm64v8/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        docker_platform="--platform linux/arm64"
        ;;
    "arm32"|"armhf")
        ait_arch="armhf"
        docker_tag="arm32v7/${docker_base}:${docker_dist}"
        docker_image="${docker_tag}"-"${docker_arch}"
        docker_platform="--platform linux/arm/v7"
        ;;
    *)
        echo "Unknown architecture: $LP3D_ARCH"
        exit 3
        ;;
esac

# format the log name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# make sure we're in the repository root directory
cd ${GITHUB_WORKSPACE}

# prepare output directory
[ ! -d "${LP3D_BUILDPKG_PATH}" ] && \
mkdir -p ${LP3D_BUILDPKG_PATH} || :
out_path="${LP3D_BUILDPKG_PATH}"

# prepare third-party distribution directory
[ ! -d "${LP3D_3RD_PARTY_PATH}/${LP3D_BASE}_${LP3D_ARCH}" ] && \
mkdir -p ${LP3D_3RD_PARTY_PATH}/${LP3D_BASE}_${LP3D_ARCH} || :
base_path="${LP3D_3RD_PARTY_PATH}/${LP3D_BASE}_${LP3D_ARCH}"

# automatic logging
WRITE_LOG=${WRITE_LOG:-true}
if [ "${WRITE_LOG}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"; f="${f}-${docker_base}-${docker_dist}-${docker_arch}"
    [ "${LP3D_APPIMAGE}" = "true" ] && f="${f}-appimage"
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
    echo "LDraw library extracted to ${ldraw_path}"
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
[[ "$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')" == *"BUILD_LDGLITE"* ]] && \
echo "'Build LDGLite' detected in environment variable." && [ -d "${ldglite_path}" ] && \
rm -rf "${ldglite_path}" && echo "cached ${ldglite_path} deleted" || :
[[ "$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')" == *"BUILD_LDVIEW"* ]] && \
echo "'Build LDView' detected in environment variable." && [ -d "${ldview_path}" ] && \
rm -rf "${ldview_path}" && echo "cached ${ldview_path} deleted" || :
[[ "$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')" == *"BUILD_POVRAY"* ]] && \
echo "'Build POV-Ray' detected in environment variable." && [ -d "${povray_path}" ] && \
rm -rf "${povray_path}" && echo "cached ${povray_path} deleted" || :

# run builds with privileged user account required to load dependent
gid="$(id -g)"
uid="$(id -u)"
name="$(id -un)"

# generate Dockerfile and build image
cat << pbEOF >${out_path}/Dockerfile
FROM ${docker_tag}
pbEOF
case "${docker_base}" in
    "ubuntu")
cat << pbEOF >>${out_path}/Dockerfile
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y apt-utils git wget cmake rsync unzip lintian build-essential debhelper fakeroot ccache lsb-release
RUN apt-get install -y libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev
RUN apt-get install -y autotools-dev autoconf pkg-config automake libtool curl zip fuse libfuse-dev
RUN apt-get install -y xvfb desktop-file-utils
RUN apt-get install -y $(grep Build-Depends control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,)
pbEOF
        ;;
    "fedora")
cat << pbEOF >>${out_path}/Dockerfile
ADD lpub3d.spec /
RUN dnf install -y git wget unzip rsync which rpmlint ccache dnf-plugins-core rpm-build fuse fuse-devel
RUN dnf install -y glib2-devel libfdt-devel pixman-devel zlib-devel bzip2 ninja-build python3
RUN dnf install -y xorg-x11-server-Xvfb desktop-file-utils
RUN dnf builddep -y /lpub3d.spec
pbEOF
        ;;
    "archlinux")
cat << pbEOF >>${out_path}/Dockerfile
RUN pacman -Suy --noconfirm
# WORKAROUND for glibc 2.33 and old Docker
# See https://github.com/actions/virtual-environments/issues/2658
# Thanks to https://github.com/lxqt/lxqt-panel/pull/1562
RUN patched_glibc=glibc-linux4-2.33-4-x86_64.pkg.tar.zst && \\
    curl -LO "https://repo.archlinuxcn.org/x86_64/\$patched_glibc" && \\
    bsdtar -C / -xvf "\$patched_glibc"
RUN pacman -Sy --noconfirm git wget unzip rsync ccache base-devel binutils fakeroot tinyxml awk file inetutils
RUN pacman -S --noconfirm --needed xorg-server-xvfb desktop-file-utils
RUN pacman -S --noconfirm --needed $(grep depends PKGBUILD | cut -f2 -d=|tr -d \'\(\) | tr '\n' ' ')
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
VOLUME ["/out", "/dist", "/${name}/ldraw"]
pbEOF

# If QEMU and not AppImage, set package build script
if [[ "${LP3D_APPIMAGE}" = "false" && "${LP3D_QEMU}" = "true" ]]; then
    case "${docker_base}" in
        "ubuntu")
            cp -f builds/linux/CreateDeb.sh .
            cp -f builds/linux/obs/alldeps/debian/control .
cat << pbEOF >>${out_path}/Dockerfile
RUN mkdir -p /${name}/debbuild/SOURCES
ADD CreateDeb.sh /${name}
pbEOF
            ;;
        "fedora")
            cp -f builds/linux/CreateRpm.sh .
            cp -f builds/linux/obs/lpub3d.rpmlintrc . && \
            cp -f builds/linux/obs/alldeps/lpub3d.spec . && \
            sed -e 's/Icon: lpub3d.xpm/# Icon: lpub3d.xpm remarked - fedora does not like/' \
                -e 's/<B_CNT>/1/' -i lpub3d.spec
cat << pbEOF >>${out_path}/Dockerfile
RUN mkdir -p /${name}/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
ADD CreateRpm.sh /${name}
ADD lpub3d.spec /${name}
ADD lpub3d.rpmlintrc /${name}/.config/rpmlint/
pbEOF
            ;;
       "archlinux")
            cp -f builds/linux/CreatePkg.sh .
            cp -f builds/linux/obs/alldeps/PKGBUILD .
cat << pbEOF >>${out_path}/Dockerfile
RUN mkdir -p /${name}/pkgbuild/{src,upstream}
ADD CreatePkg.sh /${name}
pbEOF
            ;;
    esac
fi

cat << pbEOF >>${out_path}/Dockerfile
ADD docker-run-CMD.sh /${name}
CMD /bin/bash
pbEOF

cat << pbEOF >${out_path}/docker-run-CMD.sh
#!/bin/bash
cd ~/ \\
&& wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateLinuxPkg.sh \\
&& chmod a+x CreateLinuxPkg.sh \\
&& ./CreateLinuxPkg.sh \\
&& if test -d /out; then \\
  cd ~/; \\
  ls -al ./ 2>/dev/null || :; \\
  ls -al /out/ 2>/dev/null || :; \\
  ls -al /dist/ 2>/dev/null || :; \\
fi
pbEOF

# add Dockerfile to context
cp -f ${out_path}/Dockerfile .

# add run CMD script to context and set executable
cp -f ${out_path}/docker-run-CMD.sh . && chmod a+x docker-run-CMD.sh

# Docker Hub login
echo "Login to Docker Hub..."
echo ${DOCKER_HUB_TOKEN} | docker login --username ${DOCKER_USERNAME} --password-stdin

# Enable QEMU
if [ "${LP3D_QEMU}" = "true" ]; then
    # Setup QEMU multiarch environment using the runners shell
    echo "Enable QEMU multiarch environment..."
    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

    # Build appimagetool
    if [ "${LP3D_APPIMAGE}" = "true" ]; then
        if [ ! -f "${base_path}/appimagetool" ]; then
            p=AppImageTool
            echo Building $p for ${ait_arch}...
            git clone --single-branch --recursive https://github.com/AppImage/AppImageKit $p
            ( cd $p && \
              export ARCH=${ait_arch} && \
              chmod a+x ci/build.sh && ./ci/build.sh && \
              ( [ -n "$(ls -A ./out/)" ] && mv -f ./out/* ${base_path}/ ) \
            ) >$p.out 2>&1 && mv $p.out $p.ok
            if [ -f $p.ok ]; then
                echo Build $p succeeded
                mv -f $p.ok ${base_path}
                ( cd ${base_path} && \
                  ait=appimagetool-${ait_arch}.AppImage && \
                  ( [ -f "${ait}" ] && mv ${ait} appimagetool && chmod a+x appimagetool ) \
                ) >$p.out 2>&1 && mv $p.out $p.ok
                if [ -f $p.ok ]; then
                    cat $p.ok >> ${base_path}/$p.ok && rm $p.ok
                    ait=${base_path}/appimagetool
                    ( qemu-${ait_arch}-static -L /usr/${ait_arch}-linux-gnu ${ait} -version \
                    ) >$p.out 2>&1 && mv $p.out $p.ok
                    if [ ! -f $p.ok ]; then
                        mb="$(hexdump -Cv ${ait} | head -n 1 | grep '41 49 02 00')"
                        if [ -n "${mb}" ]; then
                            echo "$p magic bytes: ${mb}"
                            echo "Patching $p magic bytes..."
                            dd if=/dev/zero of="${ait}" bs=1 count=3 seek=8 conv=notrunc
                            if [ -z "$(hexdump -Cv ${ait} | head -n 1 | grep '41 49 02 00')" ]; then
                                echo "$p magic bytes patched $(hexdump -Cv ${ait} | head -n 1)"
                                ( qemu-${ait_arch}-static -L /usr/${ait_arch}-linux-gnu ${ait} -version \
                                ) >$p.out 2>&1 && mv $p.out $p.ok
                                if [ -f $p.ok ]; then
                                    cat $p.ok
                                    cat $p.ok >> ${base_path}/$p.ok && rm $p.ok
                                    echo $p is runnable
                                else
                                    echo Test run $p FAILED
                                    tail -80 $p.out
                                    # exit 9
                                fi
                            else
                                echo Patch $p magic bytes FAILED
                                hexdump -Cv ${ait} | head -n 3
                                exit 9
                            fi
                        else
                            echo "Magic bytes 'AI' not found in $p"
                            hexdump -Cv ${ait} | head -n 3
                        fi
                    else
                        cat $p.ok
                        cat $p.ok >> ${base_path}/$p.ok && rm $p.ok
                        echo $p is runnable
                    fi
                else
                    echo Build $p FAILED
                    tail -80 $p.out
                    exit 9
                fi
            fi
        else
            echo Build $p FAILED
            tail -80 $p.out
            exit 9
        fi
        # Run pre-package build check default is Off for AppImage builds
        [[ "$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')" == *"PRE_BUILD_CHECK"* ]] && \
        LP3D_PRE_PACKAGE_CHECK=1 || unset LP3D_PRE_PACKAGE_CHECK
    else
        # Run pre-package build check default is On for package builds
        [[ "$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')" == *"NO_PRE_BUILD_CHECK"* ]] && \
        unset LP3D_PRE_PACKAGE_CHECK || LP3D_PRE_PACKAGE_CHECK=1
    fi

    # replace magic bytes
    [[ "$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')" == *"AI_MAGIC_BYTES"* ]] && \
    unset LP3D_AI_MAGIC_BYTES || LP3D_AI_MAGIC_BYTES=1:
fi

# build docker image
docker build --rm \
    ${docker_platform} \
    --pull \
    --no-cache \
    -f Dockerfile \
    -t ${docker_image} .

# marshall docker run options
common_docker_opts=(
    -e CI="${CI}"
    -e GITHUB="${GITHUB}"
    -e DOCKER="${DOCKER:-true}"
    -e TERM="${TERM}"
    -e GITHUB_SHA="${GITHUB_SHA}"
    -e WRITE_LOG="${WRITE_LOG}"
    -e BUILD_OPT="${BUILD_OPT}"
    -e BUILD_CPUs="${BUILD_CPUs}"
    -e LP3D_QEMU="${LP3D_QEMU}"
    -e LP3D_BASE="${LP3D_BASE}"
    -e LP3D_ARCH="${LP3D_ARCH}"
    -e LP3D_APPIMAGE="${LP3D_APPIMAGE}"
    -e LP3D_NO_CLEANUP="${LP3D_NO_CLEANUP}"
    -e LP3D_AI_MAGIC_BYTES="${LP3D_AI_MAGIC_BYTES}"
    -e LP3D_PRE_PACKAGE_CHECK="${LP3D_PRE_PACKAGE_CHECK}"
    -v "${out_path}":/out
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

# list the built files
test -d ${out_path} && ls -al ${out_path} || :

exit 0
