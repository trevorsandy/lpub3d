#!/bin/bash
# Trevor SANDY
# Last Update: September 19, 2025
# Copyright (C) 2017 - 2025 by Trevor SANDY
# Build LPub3D Linux deb distribution
# To run:
# $ chmod 755 CreateDeb.sh
# $ [options] && ./builds/linux/CreateDeb.sh
# [options]:
#  - LOCAL=false - local build - use local versus download renderer and library source
#  - DOCKER=true - using Docker image
#  - LPUB3D=lpub3d - repository name
#  - OBS=false - building locally
#  - LP3D_ARCH=amd64 - set build architecture
#  - LP3D_BASE=ubuntu - set build base OS
#  - PRESERVE=false - clone remote repository
#  - UPDATE_SH=false - update overwrite this script when building in local Docker
#  - DEB_EXTENSION=amd64.deb - distribution file suffix
#  - LOCAL_RESOURCE_PATH= - path (or Docker volume mount) where lpub3d and renderer sources and library archives are located
#  - XSERVER=false - use Docker host XMing/XSrv XServer
#  - LPUB3D_BRANCH=master - repository branch to be built
#  - LP3D_LOG_PATH=~/logs - log path
# NOTE: elevated access required for apt-get install, execute with sudo
# or enable user with no password sudo if running noninteractive - see
# docker-compose/dockerfiles for script example of sudo, no password user.

# LOCAL DOCKER RUN - set accordingly then cut and paste in console to run:
: <<'BLOCK_COMMENT'
UPDATE_SH="${UPDATE_SH:-true}"
if test "${UPDATE_SH}" = "true"; then \
cp -rf /user/resources/builds/linux/CreateDeb.sh . \
&& export LOCAL=true \
&& export DOCKER=true \
&& export LPUB3D=lpub3d \
&& export PRESERVE=true \
&& export LP3D_ARCH=amd64 \
&& export LP3D_BASE=ubuntu \
&& export LOCAL_RESOURCE_PATH=/user/resources \
&& export LPUB3D_BRANCH=master \
&& export LP3D_LOG_PATH=${HOME}/logs \
&& export XSERVER=false \
&& chmod a+x CreateDeb.sh \
&& ./CreateDeb.sh \
&& if test -d /buildpkg; then \
  sudo cp -f /user/rpmbuild/RPMS/`uname -m`/*.rpm /buildpkg/; \
  sudo cp -f /user/rpmbuild/BUILD/*.log /buildpkg/; \
  sudo cp -f /user/*.log /buildpkg/; \
fi

BLOCK_COMMENT

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  ME="${ME} for (${LP3D_BASE}-${LP3D_ARCH})"
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "$ME Verification Finished!"
  else
    echo "$ME Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

CWD=`pwd`

# Change thse when you change the LPub3D root directory (e.g. if using a different root folder when testing)
OBS=${OBS:-false}
LOCAL=${LOCAL:-}
LPUB3D=${LPUB3D:-lpub3d}
DOCKER=${DOCKER:-}
XSERVER=${XSERVER:-}
PRESERVE=${PRESERVE:-} # preserve cloned repository
LP3D_ARCH=${LP3D_ARCH:-amd64}
LP3D_BASE=${LP3D_BASE:-ubuntu}
DEB_EXTENSION=${DEB_EXTENSION:-$LP3D_ARCH.deb}
LOCAL_RESOURCE_PATH=${LOCAL_RESOURCE_PATH:-}
LPUB3D_BRANCH=${LPUB3D_BRANCH:-master}
LDGLITE_BRANCH=${LDGLITE_BRANCH:-master}
LDVIEW_BRANCH=${LDVIEW_BRANCH:-lpub3d-build}
POVRAY_BRANCH=${POVRAY_BRANCH:-lpub3d/raytracer-cui}
LP3D_PUBLISH_RENDERERS=${LP3D_PUBLISH_RENDERERS:-false}
LP3D_GITHUB_URL="https://github.com/trevorsandy"
CMD_CNT=0

export OBS # OpenSUSE Build Service flag must be set for CreateRenderers.sh

# Format name and set WRITE_LOG - SOURCED if $1 is empty
ME="CreateDeb"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

# automatic logging
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
if [ "${WRITE_LOG}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"; [ -n "${LP3D_ARCH}" ] && f="${f}-${LP3D_ARCH}" || f="${f}-amd64"
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

export LP3D_LOG_PATH

if [ "${XSERVER}" = "true" ]; then
    if test "${LOCAL}" != "true"; then export XSERVER=; fi
fi

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# when running on Travis-CI, use this block...
if [ "${TRAVIS}" = "true"  ]; then
    # Travis starting point: /home/travis/build/trevorsandy/lpub3d
    # Travis starts in the clone directory (lpub3d/), so move outside
    cd ../
fi

echo "Start $ME execution at $CWD..."

echo "   LPUB3D SOURCE DIR........${LPUB3D}"
echo "   LPUB3D BUILD ARCH........${LP3D_ARCH}"
echo "   LPUB3D BRANCH_LPUB3D.....${LPUB3D_BRANCH}"
echo "   LPUB3D BRANCH_LDGLITE....${LDGLITE_BRANCH}"
echo "   LPUB3D BRANCH_LDVIEW.....${LDVIEW_BRANCH}"
echo "   LPUB3D BRANCH_POVRAY.....${POVRAY_BRANCH}"
echo "   LPUB3D BUILD PLATFORM....$([ "$DOCKER" = "true" ] && echo "DOCKER" || echo "HOST RUNNER")"
if [ "$LOCAL" = "true" ]; then
    echo "   LPUB3D BUILD TYPE........Local"
    echo "   LPUB3D BUILD DISPLAY.....$(if test "${XSERVER}" = "true"; then echo XSERVER; else echo XVFB; fi)"
    echo "   UPDATE BUILD SCRIPT......$(if test "${UPDATE_SH}" = "true"; then echo YES; else echo NO; fi)"
    if [ -n "$LOCAL_RESOURCE_PATH" ]; then
        echo "   LOCAL_RESOURCE_PATH......${LOCAL_RESOURCE_PATH}"
    else
        echo "ERROR - LOCAL_RESOURCE_PATH was not specified. $ME will terminate."
        exit 1
    fi
else
    echo "   LPUB3D BUILD TYPE........CI"
fi
echo "   PRESERVE BUILD REPO......$(if test "${PRESERVE}" = "true"; then echo YES; else echo NO; fi)"
[ -n "${LP3D_PUBLISH_RENDERERS}" ] && \
echo "   PUBLISH RENDERERS........${LP3D_PUBLISH_RENDERERS}" || :
echo "   LOG PATH.................${LP3D_LOG_PATH}"

echo "$((CMD_CNT+=1)). create DEB working directories in debbuild/..."
SOURCE_DIR=debbuild/SOURCES
if [ ! -d "${SOURCE_DIR}" ]
then
    mkdir -p "${SOURCE_DIR}"
fi

l=Log
cd debbuild/
BUILD_DIR=$PWD
cd ${BUILD_DIR}/SOURCES

if [ "${TRAVIS}" != "true" ]; then
    if [ -d "/in" ]; then
        echo -n "$((CMD_CNT+=1)). copy input source to ${SOURCE_DIR}/${LPUB3D}..."
        (mkdir -p ${LPUB3D} && cp -rf /in/. ${LPUB3D}/) >$l.out 2>&1 && rm $l.out
		[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
    else
        LPUB3D_REPO=$(find . -maxdepth 1 -type d -name "${LPUB3D}-*")
        if [[ "${PRESERVE}" != "true" || ! -d "${LPUB3D_REPO}" ]]; then
            if [ "$LOCAL" = "true" ]; then
                echo -n "$((CMD_CNT+=1)). copy LOCAL ${LPUB3D} source to ${SOURCE_DIR}/..."
                (cp -rf ${LOCAL_RESOURCE_PATH}/${LPUB3D} ${LPUB3D}) >$l.out 2>&1 && rm $l.out
				[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
                echo "$((CMD_CNT+=1)). copy LOCAL ${LPUB3D} renderer source to ${SOURCE_DIR}/"
                for renderer in ldglite ldview povray; do
				    echo -n "   copy LOCAL ${renderer} source..."
                    (cp -rf ${LOCAL_RESOURCE_PATH}/${renderer}.tar.gz .) >$l.out 2>&1 && rm $l.out
					[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
                done
            else
                echo "$((CMD_CNT+=1)). download ${LPUB3D} source to ${SOURCE_DIR}/"
                if [ -d "${LPUB3D_REPO}" ]; then
				    echo "$((CMD_CNT+=1)). remove old ${LPUB3D_REPO} from ${SOURCE_DIR}/"
                    rm -rf ${LPUB3D_REPO}
                fi
                echo -n "$((CMD_CNT+=1)). cloning ${LPUB3D} ${LPUB3D_BRANCH} branch into ${LPUB3D}..."
                (git clone -b ${LPUB3D_BRANCH} ${LP3D_GITHUB_URL}/${LPUB3D}.git) >$l.out 2>&1 && rm $l.out
                [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
            fi
        else
            echo "$((CMD_CNT+=1)). preserve ${LPUB3D} source in ${SOURCE_DIR}/"
            if [ -d "${LPUB3D_REPO}" ]; then
                echo "$((CMD_CNT+=1)). move ${LPUB3D_REPO} to ${LPUB3D} in ${SOURCE_DIR}/"
                mv -f ${LPUB3D_REPO} ${LPUB3D}
            fi
        fi
    fi
else
    echo -n "$((CMD_CNT+=1)). copy ${LPUB3D} source to ${SOURCE_DIR}/..."
    (cp -rf "../../${LPUB3D}" .) >$l.out 2>&1 && rm $l.out
	[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
fi

# For Docker build, check if there is a tag after the last commit
if [ "$DOCKER" = "true" ]; then
   # Setup git command
   GIT_CMD="git --git-dir $PWD/${LPUB3D}/.git --work-tree $PWD/${LPUB3D}"
   # Update source
   [ "${CI}" = "true" ] && $GIT_CMD pull || :
   #1. Get the latest version tag - check across all branches
   BUILD_TAG=$($GIT_CMD describe --tags --match v* $($GIT_CMD rev-list --tags --max-count=1) 2> /dev/null)
   if [ -n "$BUILD_TAG" ]; then
       #2. Get the tag datetime
       BUILD_TAG_TIME=$($GIT_CMD log -1 --format=%ai $BUILD_TAG 2> /dev/null)
       #3. Get the latest commit datetime from the build branch
       GIT_COMMIT_TIME=$($GIT_CMD log -1 --format=%ai 2> /dev/null)
       #4. If tag is newer than commit, check out the tag
       if [ $(date -d "$GIT_COMMIT_TIME" +%s) -lt $(date -d "$BUILD_TAG_TIME" +%s) ]; then
           echo -n "$((CMD_CNT+=1)). checking out newer build tag $BUILD_TAG..."
           ($GIT_CMD checkout -qf $BUILD_TAG) >$l.out 2>&1 && rm $l.out
           [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
       fi
   fi
fi

echo "$((CMD_CNT+=1)). source update_config_files.sh"
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

WORK_DIR=${LPUB3D}-${LP3D_APP_VERSION}
if [[ "${PRESERVE}" != "true" || ! -d ${WORK_DIR} ]]; then
    if [ -d ${WORK_DIR} ]; then
	    echo "$((CMD_CNT+=1)). remove old ${WORK_DIR} from ${SOURCE_DIR}/"
        rm -rf ${WORK_DIR}
    fi
	echo "$((CMD_CNT+=1)). move ${LPUB3D}/ to ${LPUB3D}-${LP3D_APP_VERSION}/ in ${SOURCE_DIR}/"
    mv -f ${LPUB3D} ${WORK_DIR}
else
    if [ "$LOCAL" = "true" ]; then
        echo -n "$((CMD_CNT+=1)). overwrite ${LPUB3D}-${LP3D_APP_VERSION}/ with ${LPUB3D}/ in ${SOURCE_DIR}/..."
        (cp -TRf ${LPUB3D}/ ${WORK_DIR}/ && \
        rm -rf ${LPUB3D}) >$l.out 2>&1 && rm $l.out
        [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
    else
        echo "$((CMD_CNT+=1)). preserve ${LPUB3D}-${LP3D_APP_VERSION}/ in ${SOURCE_DIR}/"
    fi
fi

echo "$((CMD_CNT+=1)). create cleaned tarball ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz from ${WORK_DIR}/"
tar -czf ../${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz \
        --exclude=".gitignore" \
        --exclude=".gitattributes" \
        --exclude=".travis.yml" \
        --exclude="LPub3D.pro.user" \
        --exclude="appveyor.yml" \
        --exclude="README.md" \
        --exclude="builds/utilities/Copyright-Source-Header.txt" \
        --exclude="builds/utilities/create-dmg" \
        --exclude="builds/utilities/CreateRenderers.bat" \
        --exclude="builds/utilities/README.md" \
        --exclude="builds/utilities/set-ldrawdir.command" \
        --exclude="builds/utilities/update-config-files.bat" \
        --exclude="builds/utilities/cert" \
        --exclude="builds/utilities/ci" \
        --exclude="builds/utilities/dmg-utils" \
        --exclude="builds/utilities/hooks" \
        --exclude="builds/utilities/icons" \
        --exclude="builds/utilities/json" \
        --exclude="builds/utilities/nsis-scripts" \
        --exclude="builds/linux/docker-compose" \
        --exclude="builds/linux/standard" \
        --exclude="builds/linux/CreateLinux.sh" \
        --exclude="builds/linux/CreatePkg.sh" \
        --exclude="builds/linux/CreateRpm.sh" \
        --exclude="builds/windows" \
        --exclude="builds/macx" \
        --exclude="lclib/tools" \
        ${WORK_DIR}

# we pull in the library archives here because the lpub3d.spec file copies them
# to the extras location. This config thus supports both Suse OBS and Travis CI build procs.
ldrawLibFiles=(complete.zip lpub3dldrawunf.zip tenteparts.zip vexiqparts.zip)
if [[ "$LOCAL" = "true" || "$PRESERVE" = "true" ]]; then
    LOCAL_LDRAW_PATH=${LOCAL_RESOURCE_PATH}/ldraw-libraries
    [ ! -d "${LOCAL_LDRAW_PATH}" ] && unset LOCAL_LDRAW_PATH || :
fi
if [ -n "${LOCAL_LDRAW_PATH}" ]; then
    echo "$((CMD_CNT+=1)). copy LDraw archive libraries to ${SOURCE_DIR}/"
    for libFile in "${ldrawLibFiles[@]}"; do
        if [ ! -f "${libFile}" ]; then
            echo -n "   copying ${libFile} into ${SOURCE_DIR}/..."
            (cp -rf ${LOCAL_LDRAW_PATH}/${libFile} .) >$l.out 2>&1 && rm $l.out
            [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
        fi
    done
else
    echo "$((CMD_CNT+=1)). download LDraw archive libraries to ${SOURCE_DIR}/"
    LP3D_LIBS_BASE="${LP3D_GITHUB_URL}/lpub3d_libs/releases/download/v1.0.1"
    for libFile in "${ldrawLibFiles[@]}"; do
        if [ ! -f "${libFile}" ]; then
            echo -n "   downloading ${libFile} into ${SOURCE_DIR}/..."
            (curl $curlopts ${LP3D_LIBS_BASE}/${libFile} -o ${libFile}) >$l.out 2>&1 && rm $l.out
            [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
        fi
    done
fi

if [ -d ../lpub3d_linux_3rdparty ]; then
    echo "$((CMD_CNT+=1)). linking LDraw archive libraries in 3rdParty folder"
    for libFile in ${libFile} complete.zip; do
        echo -n "   linking ${libFile}..."
        (cd ../lpub3d_linux_3rdparty && ln -sf ../SOURCES/${libFile} ${libFile}) >$l.out 2>&1 && rm $l.out
        [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
    done
fi

# download 3rd party packages defined as source in pkgbuild/
dwMsgShown=0
packageFiles=(ldglite ldview povray)
for pkgFile in "${packageFiles[@]}"; do
    ext=tar.gz
    package="$(echo ${pkgFile} | awk '{print toupper($0)}')"
    if [ ! -f "${pkgFile}.${ext}" ]; then
        if [ "$LOCAL" = "true" ]; then
            [ "${dwMsgShown}" -eq 0 ] && \
            echo "$((CMD_CNT+=1)). copy ${LPUB3D} package source to ${SOURCE_DIR}/" || :
			echo -n "   copying LOCAL ${package} tarball ${pkgFile}.${ext}..."
            (cp -f ${LOCAL_RESOURCE_PATH}/${pkgFile}.${ext} .) >$l.out 2>&1 && rm $l.out
			[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
        elif [[ "$PRESERVE" = "true" && -f ../${pkgFile}.${ext} ]]; then
            [ "${dwMsgShown}" -eq 0 ] && \
            echo "$((CMD_CNT+=1)). copy ${LPUB3D} package source to ${SOURCE_DIR}/" || :
            echo -n "   copying PRESERVE ${package} tarball ${pkgFile}.${ext}..."
            (cp -f ../${pkgFile}.${ext} .) >$l.out 2>&1 && rm $l.out
			[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
        else
            [ "${dwMsgShown}" -eq 0 ] && \
            echo "$((CMD_CNT+=1)). download ${LPUB3D} package source to ${SOURCE_DIR}/" || :
            case "${pkgFile}" in
                ldglite) buildBranch=${LDGLITE_BRANCH} ;;
                ldview) buildBranch=${LDVIEW_BRANCH} ;;
                povray) buildBranch=${POVRAY_BRANCH} ;;
            esac
            curlCommand="${LP3D_GITHUB_URL}/${pkgFile}/archive/${buildBranch}.${ext}"
            echo -n "   downloading ${package} tarball ${pkgFile}.${ext}..."
            (curl $curlopts ${curlCommand} -o ${pkgFile}.${ext}) >$l.out 2>&1 && rm $l.out
            [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
        fi
        dwMsgShown=1
    fi
done

echo "$((CMD_CNT+=1)). extract ${WORK_DIR}/ to debbuild/"
cd ${BUILD_DIR}/
if [  -d ${LPUB3D}-${LP3D_APP_VERSION} ]
then
   rm -rf ${LPUB3D}-${LP3D_APP_VERSION}
fi
tar zxf ${LPUB3D}_${LP3D_APP_VERSION}.orig.tar.gz

echo -n "$((CMD_CNT+=1)). copy debian/ configuration directory to ${WORK_DIR}/..."
(cp -rf ${WORK_DIR}/builds/linux/obs/debian ${WORK_DIR}) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

LP3D_PLATFORM_ID=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $(uname) | awk '{print tolower($0)}')
LP3D_PLATFORM_VER=$(. /etc/os-release 2>/dev/null; [ -n "$VERSION_ID" ] && echo $VERSION_ID || true)
LP3D_DEBIAN_VER=$([ $(echo ${LP3D_PLATFORM_ID} | awk '{print tolower($$0)}') = "debian" ] && [ "${LP3D_PLATFORM_VER}" = "10" ] && echo 10 || :)
if [[ "${LP3D_DEBIAN_VER}" -eq 10 || "$([ $(which ucslint) ] && echo 5 || :)" -eq 5 ]]; then
  echo "$((CMD_CNT+=1)). set debian/compat to 9"
  echo 9 > ${WORK_DIR}/debian/compat
fi

cd "${BUILD_DIR}/${WORK_DIR}/"

if [ "${CI}" = "true" ]; then
    echo "$((CMD_CNT+=1)). skipping install ${LPUB3D} build dependencies."
else
    echo "$((CMD_CNT+=1)). install ${LPUB3D} build dependencies [requires elevated access - sudo]..."
    controlDeps=`grep Build-Depends debian/control | cut -d: -f2| sed 's/(.*)//g' | tr -d ,`
    sudo apt-get update -qq
    sudo apt-get install -y $controlDeps
fi

if [ "$GITHUB" = "true" ]; then
    export DEBUILD_PRESERVE_ENVVARS="CI,OBS,GITHUB,DOCKER,LP3D_*"
fi
echo "$((CMD_CNT+=1)). build  LPub3D DEB application package..."
chmod 755 debian/rules
/usr/bin/dpkg-buildpackage -us -uc || exit 1

if [ "$LOCAL" = "true" ]; then
    cd ../
else
    cd ${BUILD_DIR}/
fi

DISTRO_FILE=$(ls ${LPUB3D}_${LP3D_APP_VERSION}*.deb)
if [[ -f ${DISTRO_FILE} ]]
then
    echo "$((CMD_CNT+=1)). build package: $PWD/${DISTRO_FILE}"
    if [[ -n "$LP3D_SKIP_BUILD_CHECK" ]]; then
        echo "$((CMD_CNT+=1)). skipping ${DISTRO_FILE} build check"
    else
        if [[ -n "$LP3D_PRE_PACKAGE_CHECK" ]]; then
            echo "$((CMD_CNT+=1)). perform ${LPUB3D} pre-package build check"
            export LP3D_BUILD_OS=
            export SOURCE_DIR=${BUILD_DIR}/${WORK_DIR}
            export LP3D_CHECK_LDD="1"
            export LP3D_CHECK_STATUS="--version --app-paths"
            case ${LP3D_ARCH} in
                "amd64"|"arm64"|"x86_64"|"aarch64")
                    LP3D_BUILD_ARCH="64bit_release" ;;
                *)
                    LP3D_BUILD_ARCH="32bit_release" ;;
            esac
            export LPUB3D_EXE="${SOURCE_DIR}/mainApp/${LP3D_BUILD_ARCH}/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
            cd ${SOURCE_DIR} && source builds/check/build_checks.sh
        else
            echo "$((CMD_CNT+=1)). build check ${DISTRO_FILE}"
            if [[ ! -f "/usr/bin/update-desktop-database" ]]; then
                echo "   install program update-desktop-database not found"
                sudo apt-get install -y desktop-file-utils
            fi
            # Install package - here we use the distro file name
            echo "   install ${LPUB3D}..."
            sudo dpkg -i ${DISTRO_FILE}
            # Check if exe exist - here we use the executable name
            LPUB3D_EXE=lpub3d${LP3D_APP_VER_SUFFIX}
            SOURCE_DIR=${WORK_DIR}
            if [[ -f "/usr/bin/${LPUB3D_EXE}" ]]; then
                # Check commands
                source ${WORK_DIR}/builds/check/build_checks.sh
                # Cleanup - here we use the package name
                echo "   uninstall ${LPUB3D}..."
                sudo dpkg -r ${LPUB3D}
            else
                echo "WARNING - build check failed - /usr/bin/${LPUB3D_EXE} not found."
            fi
        fi
    fi

    echo "$((CMD_CNT+=1)). lintian check ${DISTRO_FILE}..."
    lintian ${DISTRO_FILE} ${WORK_DIR}/${LPUB3D}.dsc

    echo "$((CMD_CNT+=1)). moving ${LP3D_BASE} ${LP3D_ARCH} assets to output folder"
    mv -f ${SOURCE_DIR}/*.log /out/ 2>/dev/null || :
    mv -f ${CWD}/*.log /out/ 2>/dev/null || :
    mv -f ./*.log /out/ 2>/dev/null || :
    mv -f ~/*.log /out/ 2>/dev/null || :
    mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :

    # Stop here if build option is verification only
    if [[ "$BUILD_OPT" = "verify" ]]; then
        echo "$((CMD_CNT+=1)). cleanup build assets"
        rm -f ./*.deb* 2>/dev/null || :
        rm -f ./*.xz 2>/dev/null || :
        exit 0
    fi

    IFS=_ read DEB_NAME DEB_VERSION DEB_EXTENSION <<< ${DISTRO_FILE}
    case ${LP3D_PLATFORM_ID} in
    ubuntu)
        case ${LP3D_PLATFORM_VER} in
        14.04) LP3D_PLATFORM_NAME="trusty" ;;
        16.04) LP3D_PLATFORM_NAME="xenial" ;;
        16.10) LP3D_PLATFORM_NAME="yakkety" ;;
        17.04) LP3D_PLATFORM_NAME="zesty" ;;
        17.10) LP3D_PLATFORM_NAME="artful" ;;
        18.04) LP3D_PLATFORM_NAME="bionic" ;;
        18.10) LP3D_PLATFORM_NAME="cosmic" ;;
        19.04) LP3D_PLATFORM_NAME="disco" ;;
        19.10) LP3D_PLATFORM_NAME="eoan" ;;
        20.04) LP3D_PLATFORM_NAME="focal" ;;
        20.10) LP3D_PLATFORM_NAME="groovy" ;;
        21.04) LP3D_PLATFORM_NAME="hirsute" ;;
        21.10) LP3D_PLATFORM_NAME="impish" ;;
        22.04) LP3D_PLATFORM_NAME="jammy" ;;
        22.10) LP3D_PLATFORM_NAME="kinetic" ;;
        23.04) LP3D_PLATFORM_NAME="lunar" ;;
        23.10) LP3D_PLATFORM_NAME="mantic" ;;
        24.04) LP3D_PLATFORM_NAME="noble" ;;
        24.10) LP3D_PLATFORM_NAME="oracular" ;;
        25.04) LP3D_PLATFORM_NAME="plucky" ;;
        *) LP3D_PLATFORM_NAME="ubuntu" ;;
        esac
        ;;
    debian)
        case ${LP3D_PLATFORM_VER} in
        8) LP3D_PLATFORM_NAME="jessie" ;;
        9) LP3D_PLATFORM_NAME="stretch" ;;
        10) LP3D_PLATFORM_NAME="buster" ;;
        11) LP3D_PLATFORM_NAME="bullseye" ;;
        12) LP3D_PLATFORM_NAME="bookworm" ;;
        13) LP3D_PLATFORM_NAME="trixie" ;;
        *) LP3D_PLATFORM_NAME="debian" ;;
        esac
        ;;
    esac;
    [[ -n "$LP3D_PLATFORM_NAME" ]] && \
    LP3D_DEB_APP_VERSION_LONG="${LP3D_APP_VERSION_LONG}-${LP3D_PLATFORM_NAME}" || \
    LP3D_DEB_APP_VERSION_LONG="${LP3D_APP_VERSION_LONG}-ubuntu"
    LP3D_DEB_FILE="LPub3D-${LP3D_DEB_APP_VERSION_LONG}-${DEB_EXTENSION}"

    echo "$((CMD_CNT+=1)). create package ${LP3D_DEB_FILE}"
    mv -f "${DISTRO_FILE}" "${LP3D_DEB_FILE}"
    if [[ -f "${LP3D_DEB_FILE}" ]]; then
        declare -r t=Trace
        echo -n "$((CMD_CNT+=1)). creating ${LP3D_DEB_FILE}.sha512 hash file..."
        ( sha512sum "${LP3D_DEB_FILE}" > "${LP3D_DEB_FILE}.sha512" ) >$t.out 2>&1 && rm $t.out
        [[ -f "$t.out" ]] && \
        echo && echo "ERROR - Failed to create sha512 file." && tail -20 $t.out || echo "Ok."
        echo -n "$((CMD_CNT+=1)). moving ${LP3D_BASE} ${LP3D_ARCH} deb build package to output folder..."
        ( for file in *.deb*; do mv -f "${file}" /out/; done ) >$t.out 2>&1 && rm $t.out
        if [[ -f "$t.out" ]]; then
            echo && echo "ERROR - move deb build package to output failed." && \
            tail -80 $t.out || echo "Ok."
        else
            mv -f ${BUILD_DIR}/*.xz /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.dsc /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.changes /out/ 2>/dev/null || :
            mv -f ${BUILD_DIR}/*.buildinfo /out/ 2>/dev/null || :
        fi
        echo
        echo "    Distribution package.: ${LP3D_DEB_FILE}"
        echo "    Package path.........: $PWD/${LP3D_DEB_FILE}"
    else
        echo "ERROR - file ${LP3D_DEB_FILE} not found"
    fi
else
    echo "ERROR - package ${DISTRO_FILE} not found"
fi

exit 0
