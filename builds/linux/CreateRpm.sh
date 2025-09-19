#!/bin/bash
# Trevor SANDY
# Last Update: September 19, 2025
# Copyright (C) 2017 - 2025 by Trevor SANDY
# Build LPub3D Linux rpm distribution
# To run:
# $ chmod 755 CreateDeb.sh
# $ [options] && ./builds/linux/CreateRpm.sh
# [options]:
#  - LOCAL=false - local build - use local versus download renderer and library source
#  - DOCKER=true - using Docker image
#  - LPUB3D=lpub3d - repository name
#  - OBS=false - building locally
#  - LP3D_ARCH=amd64 - set build architecture
#  - LP3D_BASE=fedora - set build base OS
#  - PRESERVE=false - clone remote repository
#  - UPDATE_SH=false - update overwrite this script when building in local Docker
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
cp -rf /user/resources/builds/linux/CreateRpm.sh . \
&& export LOCAL=true \
&& export DOCKER=true \
&& export LPUB3D=lpub3d \
&& export PRESERVE=true \
&& export LP3D_ARCH=amd64 \
&& export LP3D_BASE=fedora \
&& export LOCAL_RESOURCE_PATH=/user/resources \
&& export LPUB3D_BRANCH=master \
&& export LP3D_LOG_PATH=${HOME}/logs \
&& export XSERVER=false \
&& chmod a+x CreateRpm.sh \
&& ./CreateRpm.sh \
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
LP3D_BASE=${LP3D_BASE:-fedora}
LOCAL_RESOURCE_PATH=${LOCAL_RESOURCE_PATH:-}
LPUB3D_BRANCH=${LPUB3D_BRANCH:-master}
LDGLITE_BRANCH=${LDGLITE_BRANCH:-master}
LDVIEW_BRANCH=${LDVIEW_BRANCH:-lpub3d-build}
POVRAY_BRANCH=${POVRAY_BRANCH:-lpub3d/raytracer-cui}
LP3D_3RD_EXE_PREFIX=${INSTALL_PREFIX:-/usr}/bin/lpub3d/3rdParty
LP3D_PUBLISH_RENDERERS=${LP3D_PUBLISH_RENDERERS:-false}
LP3D_GITHUB_URL="https://github.com/trevorsandy"
LP3D_TARGET_ARCH=`uname -m`
CMD_CNT=0

export OBS # OpenSUSE Build Service flag must be set for CreateRenderers.sh - called by lpub3d.spec

# Format name and set WRITE_LOG - SOURCED if $1 is empty
ME="CreateRpm"
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

echo "Start $ME execution at $CWD..."

echo "   LPUB3D SOURCE DIR........${LPUB3D}"
echo "   LPUB3D BUILD ARCH........${LP3D_ARCH}"
echo "   LPUB3D BRANCH_LPUB3D.....${LPUB3D_BRANCH}"
echo "   LPUB3D BRANCH_LDGLITE....${LDGLITE_BRANCH}"
echo "   LPUB3D BRANCH_LDVIEW.....${LDVIEW_BRANCH}"
echo "   LPUB3D BRANCH_POVRAY.....${POVRAY_BRANCH}"
if [ "$LOCAL" = "true" ]; then
    echo "   LPUB3D BUILD TYPE........Local"
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

echo "$((CMD_CNT+=1)). create working directories BUILD, RPMS, SRPMS, SOURCES, and SPECS in rpmbuild/..."
if [ ! -d rpmbuild ]
then
    mkdir rpmbuild
fi

l=Log
cd rpmbuild
BUILD_DIR=$PWD
for RPM_DIR in {BUILD,RPMS,SRPMS,SOURCES,SPECS}
do
    if [ ! -d "${RPM_DIR}" ]
    then
        mkdir "${RPM_DIR}"
    fi
done
SOURCE_DIR=${BUILD_DIR}/SOURCES
cd ${SOURCE_DIR}

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
           echo -n "$((CMD_CNT+=1)). checking out build tag $BUILD_TAG..."
           ($GIT_CMD checkout -qf $BUILD_TAG) >$l.out 2>&1 && rm $l.out
           [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
       fi
   fi
fi

echo "$((CMD_CNT+=1)). source update_config_files.sh"
_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh

WORK_DIR=${LPUB3D}-git
if [[ "${PRESERVE}" != "true" || ! -d ${WORK_DIR} ]]; then
    if [ -d ${WORK_DIR} ]; then
        echo "$((CMD_CNT+=1)). remove old ${WORK_DIR} from ${SOURCE_DIR}/"
        rm -rf ${WORK_DIR}
    fi
    echo "$((CMD_CNT+=1)). move ${LPUB3D}/ to ${SOURCE_DIR}/${LPUB3D}-git"
    mv -f ${LPUB3D} ${WORK_DIR}
else
    if [ "$LOCAL" = "true" ]; then
        echo -n "$((CMD_CNT+=1)). overwrite ${LPUB3D}-git/ with ${LPUB3D}/ in ${SOURCE_DIR}/..."
        (cp -TRf ${LPUB3D}/ ${WORK_DIR}/ && \
        rm -rf ${LPUB3D}) >$l.out 2>&1 && rm $l.out
        [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
    else
        echo "$((CMD_CNT+=1)). preserve ${LPUB3D}-git/ in ${SOURCE_DIR}/"
    fi
fi

echo -n "$((CMD_CNT+=1)). copy lpub3d.xpm icon to ${SOURCE_DIR}/..."
(cp -f ${WORK_DIR}/mainApp/resources/lpub3d.xpm .) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

echo -n "$((CMD_CNT+=1)). copy ${LPUB3D}.spec to SPECS/..."
(cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}.spec ${BUILD_DIR}/SPECS) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

echo -n "$((CMD_CNT+=1)). copy ${LPUB3D}-rpmlintrc to SPECS/..."
(cp -f ${WORK_DIR}/builds/linux/obs/${LPUB3D}-rpmlintrc ${BUILD_DIR}/SPECS) >$l.out 2>&1 && rm $l.out
[ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."

# LDraw library archives
ldrawLibFiles=(complete.zip lpub3dldrawunf.zip tenteparts.zip vexiqparts.zip)
if [[ "$LOCAL" = "true" || "$PRESERVE" = "true" ]]; then
    LOCAL_LDRAW_PATH=${LOCAL_RESOURCE_PATH}/ldraw-libraries
    [ ! -d "${LOCAL_LDRAW_PATH}" ] && unset LOCAL_LDRAW_PATH || :
fi
if [ -n "${LOCAL_LDRAW_PATH}" ]; then
    echo "$((CMD_CNT+=1)). copy LDraw archive libraries to ${SOURCE_DIR}/"
    for libFile in "${ldrawLibFiles[@]}"; do
        if [ ! -f "${libFile}" ]; then
            echo -n "   copying ${libFile} into ${SOURCE_DIR}..."
            (cp -rf ${LOCAL_LDRAW_PATH}/${libFile} .) >$l.out 2>&1 && rm $l.out
            [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
        fi
    done
else
    echo "$((CMD_CNT+=1)). download LDraw archive libraries to ${SOURCE_DIR}/"
    LP3D_LIBS_BASE=${LP3D_GITHUB_URL}/lpub3d_libs/releases/download/v1.0.1
    for libFile in "${ldrawLibFiles[@]}"; do
        if [ ! -f "${libFile}" ]; then
            echo -n "   downloading ${libFile} into ${SOURCE_DIR}..."
            (curl $curlopts ${LP3D_LIBS_BASE}/${libFile} -o ${libFile}) >$l.out 2>&1 && rm $l.out
            [ -f $l.out ] && echo "failed." && tail -80 $l.out || echo "ok."
        fi
    done
fi

# download 3rd party packages defined as source in rpmbuild/SOURCES/
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

# file copy and downloads above must happen before we make the tarball
echo "$((CMD_CNT+=1)). create tarball SOURCES/${WORK_DIR}.tar.gz from SOURCES/${WORK_DIR}/..."
tar -czf ${WORK_DIR}.tar.gz \
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
        --exclude="builds/linux/CreateDeb.sh" \
        --exclude="builds/linux/CreateLinux.sh" \
        --exclude="builds/linux/CreatePkg.sh" \
        --exclude="builds/windows" \
        --exclude="builds/macx" \
        --exclude="lclib/tools" \
        ${WORK_DIR}

cd ${BUILD_DIR}/SPECS
echo "$((CMD_CNT+=1)). check ${LPUB3D}.spec with rpmlint"
source /etc/os-release && \
if [ "$ID" = "fedora" ]; then
    sed 's/Icon: lpub3d.xpm/# Icon: lpub3d.xpm remarked - fedora does not like/' -i "${BUILD_DIR}/SPECS/${LPUB3D}.spec"
fi
rpmlint ${LPUB3D}.spec

if [ "${GITHUB}" = "true" ]; then
    echo "$((CMD_CNT+=1)). Skipping install ${LPUB3D} build dependencies."
else
    echo "$((CMD_CNT+=1)). add ${LPUB3D} build dependencies [requires elevated access - sudo]"
    sudo dnf builddep -y ${LPUB3D}.spec
fi

echo -n "$((CMD_CNT+=1)). Check for libXext: "
libXextCheck=$(ldconfig -p | grep -i libXext)
if test "$libXextCheck"; then
    echo "found $libXextCheck"
    if test "$LOCAL" = "true"; then
        libXextPath=/usr/lib64
        libXextCheck=$(find $libXextPath -name libXext.so -type l)
        [ -z "$libXextCheck" ] && \
        echo -n "$((CMD_CNT+=1)). Patch Xext - Create symlink " && \
        for file in $(find $libXextPath -name 'libXext.*' -type f)
        do
        echo "for ${file##*/}" && shortlib=$file
            while extn=$(echo $shortlib | sed -n '/\.[0-9][0-9]*$/s/.*\(\.[0-9][0-9]*\)$/\1/p')
                [ -n "$extn" ]
            do
                shortlib=$(basename $shortlib $extn)
                sudo ln -fs $file $libXextPath/$shortlib
                echo "    $file $libXextPath/$shortlib"
            done
        done || \
        echo "   Notice: found symlink $libXextCheck"
    fi
else
    echo "did not find it in the usual paths"
fi

echo "$((CMD_CNT+=1)). build LPub3D RPM application package..."
rpmbuild \
--define "_topdir ${BUILD_DIR}" \
--define "_lp3d_log_path ${LP3D_LOG_PATH}" \
--define "_lp3d_cpu_cores ${LP3D_CPU_CORES}" \
--define "_lp3d_3rd_dist_dir ${LP3D_3RD_DIST_DIR}" \
--define "_lp3d_3rd_exec_dir ${LP3D_3RD_EXE_PREFIX}" \
--define "_lp3d_publish_renderers ${LP3D_PUBLISH_RENDERERS}" \
--define 'debug_package %{nil}' \
-vv -bb ${LPUB3D}.spec || exit 1

cd ${BUILD_DIR}/RPMS/${LP3D_TARGET_ARCH}

DISTRO_FILE=$(ls ${LPUB3D}-${LP3D_APP_VERSION}*.rpm)
if [[ -f ${DISTRO_FILE} ]]
then
    RPM_EXTENSION="${DISTRO_FILE##*-}"
    echo "$((CMD_CNT+=1)). build package: $PWD/${DISTRO_FILE}"
    if [[ -n "$LP3D_SKIP_BUILD_CHECK" ]]; then
        echo "   skipping ${DISTRO_FILE} build check"
    else
        if [[ -n "$LP3D_PRE_PACKAGE_CHECK" ]]; then
            echo "$((CMD_CNT+=1)). perform ${LPUB3D} pre-package build check"
            export LP3D_BUILD_OS=
            export SOURCE_DIR=$(readlink -f ../../SOURCES/${WORK_DIR})
            export LP3D_CHECK_STATUS="--version --app-paths"
            case ${LP3D_ARCH} in
                "amd64"|"arm64"|"x86_64"|"aarch64")
                    LP3D_BUILD_ARCH="64bit_release" ;;
                *)
                    LP3D_BUILD_ARCH="32bit_release" ;;
            esac
            export LPUB3D_EXE="${BUILD_DIR}/BUILD/${WORK_DIR}/mainApp/${LP3D_BUILD_ARCH}/lpub3d${LP3D_VER_MAJOR}${LP3D_VER_MINOR}"
            cd ${SOURCE_DIR} && source builds/check/build_checks.sh
        else
            echo "$((CMD_CNT+=1)). Build check ${DISTRO_FILE}"
            if [[ ! -f "/usr/bin/update-desktop-database" ]]; then
                echo "   install program update-desktop-database not found"
                sudo dnf install -y desktop-file-utils
            fi

            # Install package - here we use the distro file name e.g. LPub3D-2.3.8.1566-1.fc36.x86_64.rpm
            echo "   install ${LPUB3D}..."
            yes | sudo rpm -Uvh ${DISTRO_FILE}
            # Check if exe exist - here we use the executable name e.g. lpub3d23
            LPUB3D_EXE=lpub3d${LP3D_APP_VER_SUFFIX}
            SOURCE_DIR=../../SOURCES/${WORK_DIR}
            if [[ -f "/usr/bin/${LPUB3D_EXE}" ]]; then
                # Check commands
                LP3D_CHECK_LDD="1"
                source ${SOURCE_DIR}/builds/check/build_checks.sh
                echo "   uninstall ${LPUB3D}..."
                # Cleanup - here we use the package name e.g. lpub3d
                yes | sudo rpm -ev ${LPUB3D}
            else
                echo "WARNING - build check failed - /usr/bin/${LPUB3D_EXE} not found."
            fi
        fi
    fi

    echo "$((CMD_CNT+=1)). rpmlint check ${DISTRO_FILE}..."
    rpmlint ${DISTRO_FILE} ${LPUB3D}-${LP3D_APP_VERSION}*.rpm

    echo "$((CMD_CNT+=1)). moving ${LP3D_BASE} ${LP3D_ARCH} logs to output folder"
    mv -f ${BUILD_DIR}/BUILD/*.log 2>/dev/null || :
    mv -f ${SOURCE_DIR}/*.log /out/ 2>/dev/null || :
    mv -f ${CWD}/*.log /out/ 2>/dev/null || :
    mv -f ./*.log /out/ 2>/dev/null || :
    mv -f ~/*.log /out/ 2>/dev/null || :
    mv -f ~/*_assets.tar.gz /out/ 2>/dev/null || :

    # Stop here if build option is verification only
    if [[ "$BUILD_OPT" = "verify" ]]; then
        echo "$((CMD_CNT+=1)). cleanup build assets"
        rm -f ./*.rpm* 2>/dev/null || :
        if [[ "${GITHUB}" != "true" ]]; then
            echo "$((CMD_CNT+=1)). cleanup cloned ${LPUB3D} repository from ${SOURCE_DIR}/ and rpmbuild/BUILD/"
            rm -rf ${BUILD_DIR}/SOURCES/${WORK_DIR} ${BUILD_DIR}/BUILD/${WORK_DIR}
        fi
        exit 0
    fi

    RPM_EXTENSION="${DISTRO_FILE##*-}"
    LP3D_RPM_FILE="LPub3D-${LP3D_APP_VERSION_LONG}-${RPM_EXTENSION}"

    echo "$((CMD_CNT+=1)). create package LPub3D ${LP3D_RPM_FILE}"
    mv -f "${DISTRO_FILE}" "${LP3D_RPM_FILE}"
    if [[ -f "${LP3D_RPM_FILE}" ]]; then
        declare -r t=Trace
        echo -n "$((CMD_CNT+=1)). creating ${LP3D_RPM_FILE}.sha512 hash file..."
        ( sha512sum "${LP3D_RPM_FILE}" > "${LP3D_RPM_FILE}.sha512" ) >$t.out 2>&1 && rm $t.out
        [[ -f "$t.out" ]] && \
        echo && echo "ERROR - Failed to create sha512 file." && tail -20 $t.out || echo "Ok."
        echo -n "$((CMD_CNT+=1)). moving ${LP3D_BASE} ${LP3D_ARCH} rpm build package to output folder..."
        ( for file in *.rpm*; do mv -f "${file}" /out/; done ) >$t.out 2>&1 && rm $t.out
        [[ -f "$t.out" ]] && \
        echo && echo "ERROR - move rpm build package to output failed." && \
        tail -80 $t.out || echo "Ok."
        echo
        echo "    Distribution package.: ${LP3D_RPM_FILE}"
        echo "    Package path.........: $PWD/${LP3D_RPM_FILE}"
    else
        echo "ERROR - file ${LP3D_RPM_FILE} not found: "
    fi
else
    echo "ERROR - package ${DISTRO_FILE} not found."
fi

exit 0
