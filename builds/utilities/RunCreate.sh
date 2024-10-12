#!/bin/bash
# Trevor SANDY
# Last Update September 22, 2024
# Copyright (C) 2024 by Trevor SANDY
# Build LPub3D Linux Deb distribution
# You can run this script from the Docker image user HOME
# To run:
# $ chmod a+x RunCreate.sh
# $ env [option option ...] ./builds/utilities/RunCreate.sh [Deb]
# [optoinal arguments]:
#  - Deb - run CreateDeb.sh build script
#  - Rpm - run CreateRpm.sh build script
#  - Pkg - run CreatePkg.sh build script
# [options]:
#  - LOCAL=false - local build - use local versus download renderer and library source
#  - DOCKER=true - using Docker image
#  - LPUB3D=lpub3d - repository name
#  - LP3D_ARCH=amd64 - set build architecture
#  - PRESERVE=false - clone remote repository
#  - UPDATE_SH=false - overwrite Create script when building in local Docker
#  - DEB_EXTENSION=amd64.Deb - distribution file suffix
#  - LOCAL_RESOURCE_PATH= - path (or Docker volume mount) where lpub3d and renderer sources and library archives are located
#  - XSERVER=false - use Docker host XMing/XSrv XServer
# NOTE: elevated access required for apt-get install, execute with sudo
# or enable user with no password sudo if running noninteractive - see
# docker-compose/dockerfiles for script example of sudo, no password user.

: <<'BLOCK_COMMENT'
To run:
Configure, copy and paste the following 3 lines in the command console

RUN_CREATE_DIR=${SCRIPT_DIR:-$HOME/resources/lpub3d}
cd ~/ && cp -rf ${RUN_CREATE_DIR}/builds/utilities/RunCreate.sh . \
&& chmod a+x RunCreate.sh && ./RunCreate.sh

BLOCK_COMMENT

LOCAL=${LOCAL:-true}
DOCKER=${DOCKER:-true}
LPUB3D=${LPUB3D:-lpub3d}
PRESERVE=${PRESERVE:-true}
UPDATE_SH=${UPDATE_SH:-true}
XSERVER=${XSERVER:-false}
LP3D_ARCH=${LP3D_ARCH:-amd64}
DEB_EXT=${DEB_EXTENSION:-$LP3D_ARCH.Deb}
LOCAL_PATH=${LOCAL_RESOURCE_PATH:-$HOME/resources}

if ! test "$1"; then
  os=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID | awk '{print tolower($0)}') #'
  case $os in
    ubuntu|debian) opt=Deb ;;
    fedora|suse|centos) opt=Rpm ;;
    arch) opt=Pkg ;;
    *) echo "Could not identify the OS flavour as Debian, Redhat or Arch derived" && exit 1 ;;
  esac
else
  opt=$1
fi

case $opt in
  Deb|Rpm|Pkg) : ;;
  *) echo "Invalid option specified. Valid argument is Deb, Rpm, or Pkg." && exit 1 ;;
esac

if test "$opt" != "Deb"; then unset DEB_EXT; fi

declare -r l=run

case $opt in
  Deb) buildFolder=$HOME/debbuild ;;
  Rpm) buildFolder=$HOME/rpmbuild ;;
  Pkg) buildFolder=$HOME/pkgbuild ;;
esac

SKIP_CLEANUP=
if test "${PRESERVE}" = "true"; then
  if [ -n "$SKIP_CLEANUP" ]; then
    echo -n "Skipping up $buildFolder folder..."
  else
    echo && echo -n "Cleaning up $buildFolder folder..."
    case $opt in
      Deb)
      if test -d "$buildFolder/SOURCES"; then
        dirs="ldglite ldview povray"
        ( cd $buildFolder && \
          for dir in $dirs; do if test -d "$dir"; then mv $dir SOURCES; fi; done && \
          mv SOURCES $HOME && rm -rf ./* && mv $HOME/SOURCES . && \
          for dir in $dirs; do if test -d "SOURCES/$dir"; then mv SOURCES/$dir .; fi; done && \
          cd $HOME && rm -rf ./*.log ./*assets.tar.gz ) >$l.out 2>&1 && rm $l.out
      fi
      ;;
      Rpm)
      if test -d "$buildFolder/SOURCES"; then
        ( cd $buildFolder && \
          for dir in BUILDROOT RPMS SRPMS SPECS; do if test -d "$dir"; then rm -rf $dir; fi; done && \
          if test -d "BUILD"; then rm -rf BUILD/*.log BUILD/lpub3d*; fi && \
          cd SOURCES && if test -f "${LPUB3D}-git.tar.gz"; then tar -xf lpub3d-git.tar.gz; fi && \
          if test -d "${LPUB3D}-git"; then mv ${LPUB3D}-git ${LPUB3D}; fi && \
          rm -rf ${LPUB3D}-git.tar.gz ) >$l.out 2>&1 && rm $l.out
      fi
      ;;
      Pkg)
      if test -d "$buildFolder/src"; then
        ( cd $HOME && rm -rf *.log *assets* && \
          cd $buildFolder && if test -f "PKGBUILD"; then rm -rf PKGBUILD; fi && rm -rf pkg *.zst* *.out lpub3d*  && \
          cd $buildFolder/src && rm -rf *.log lpub3d* *.zip ) >$l.out 2>&1 && rm $l.out
      fi
      ;;
    esac
  fi

  [ -f $l.out ] && \
  echo "ERROR - Could not clean up $buildFolder folder." && \
  tail -80 $l.out && exit 1 || echo "Ok."

  if test "${UPDATE_SH}" = "true"; then
    cp -rf ${LOCAL_PATH}/${LPUB3D}/builds/linux/Create$opt.sh . && \
    chmod a+x Create$opt.sh
  fi
else
  if test -d "$buildFolder"; then rm -rf "$buildFolder"; fi
  cp -rf ${LOCAL_PATH}/${LPUB3D}/builds/linux/Create$opt.sh .
  chmod a+x Create$opt.sh
fi

env LOCAL="${LOCAL}" DOCKER="${DOCKER}" LPUB3D="${LPUB3D}" \
PRESERVE="${PRESERVE}" UPDATE_SH="${UPDATE_SH}" LP3D_ARCH="${LP3D_ARCH}" \
XSERVER="${XSERVER}" DEB_EXTENSION="${DEB_EXT}" LOCAL_RESOURCE_PATH="${LOCAL_PATH}" ./Create$opt.sh

ExportArtifacts() {
  if ! test -d "$buildFolder"; then echo "Build folder '$buildFolder' is invalid." && return 1; fi
  if ! test -d /buildpkg; then echo "Artifact folder '/buildpkg' is invalid." && return 1; fi
  fileTypeList="$fileTypeList .sha512 .log .sh assets.tar.gz"
  for fileType in $fileTypeList; do
    case $fileType in
    assets.tar.gz)
      files=$(find "$HOME" -maxdepth 1 -name "*$fileType" -type f)
      for file in $files; do sudo cp -f "$file" /buildpkg/; done
    ;;
    .log|.sh)
      f1=$(find "$buildFolder" -maxdepth 1 -name "*$fileType" -type f)
      f2=$(find "$HOME" -maxdepth 1 -name "*$fileType" -type f)
      if test "$1" = "Rpm"; then f3=$(find / -maxdepth 1 -name "*$fileType" -type f); fi
      files="$(echo "$f1 $f2 $f3" | tr " " "\n")"
      for file in $files; do sudo cp -f "$file" /buildpkg/; done
    ;;
    *)
      f4=$(find "$buildFolder" -maxdepth 1 -name "*$fileType" -type f)
      if test "$1" = "Rpm"; then f5=$(find "$buildFolder/../RPMS" -maxdepth 1 -name "*$fileType" -type f); fi
      if test "$1" = "Pkg"; then f6=$(find "$buildFolder/.." -maxdepth 1 -name *$fileType -type f); fi
      files="$(echo "$f4 $f5 $f6" | tr " " "\n")"
      for file in $files; do sudo cp -f "$file" /buildpkg/; done
    ;;
    esac
  done
}

case $opt in
  Deb)
    fileTypeList=".deb .ddeb .dsc .changes"
  ;;
  Rpm)
    fileTypeList=".rpm"
    buildFolder=$buildFolder/BUILD
  ;;
  Pkg)
    fileTypeList=".pkg.tar.zst"
    buildFolder=$buildFolder/src
  ;;
esac

echo -n "Exporting artifacts..." && \
ExportArtifacts $opt && echo "Done." && \
echo "----------------------------------------------------"
