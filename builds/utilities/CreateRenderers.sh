#!/bin/bash
#
# Build all LPub3D 3rd-party renderers
#
# Trevor SANDY <trevor.sandy@gmail.com>
# Last Update March 14, 2025
# Copyright (C) 2017 - 2025 by Trevor SANDY
#

# sample commands - called from root of repository directory - e.g. lpub3d
# $ env [environment variables] chmod +x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# $ export OBS=false; source ${SOURCE_DIR}/builds/utilities/CreateRenderers.sh
#
# Environment variables:
# WD=$PWD
# OBS=false|false
# DOCKER=true|false
# LP3D_3RD_DIST_DIR=<path relative to $WD>|lpub3d_linux_3rdparty|lpub3d_macos_3rdparty
#
# NOTES: $WD (Working Director) must be outside the repository diretory
#        OBS flag is 'ON' by default if not using DOCKER, be sure to set it false to disable in your build command accordingly
#        elevated access required for dnf builddeps, execute with sudo if running noninteractive

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# Functions
Info () {
  if [ "${SOURCED}" = "true" ]
  then
    f="${0##*/}"; f="${f%.*}"
    echo "   ${f}: ${*}" >&2
  else
    echo "-${*}" >&2
  fi
}

ExtractArchive() {
  # args: $1 = <build folder>, $2 = <valid subfolder>
  Info "Extracting $1.tar.gz..."
  mkdir -p $1 && tar -mxzf $1.tar.gz -C $1 --strip-components=1
  if [ -d $1/$2 ]; then
    Info "Archive $1.tar.gz successfully extracted."
    [ "${LP3D_NO_CLEANUP}" != "true" ] && rm -rf $1.tar.gz && Info "Cleanup archive $1.tar.gz." && Info || :
    cd $1
  else
    Info "ERROR - $1.tar.gz did not extract properly." && Info
  fi
}

BuildMesaLibs() {
  mesaUtilsDir="$CallDir/builds/utilities/mesa"
  if [ "${OBS}" != "true" ]; then
    mesaDepsLog=${LP3D_LOG_PATH}/${ME}_${host}_mesadeps_${1}.log
    mesaBuildLog=${LP3D_LOG_PATH}/${ME}_${host}_mesabuild_${1}.log
  fi
  if [ -z "$2" ]; then
    useSudo=
  else
    useSudo=$2
  fi

  Info "Update OSMesa.......[Yes]"
  if [ "${OBS}" != "true" ]; then
    case ${platform_id} in
    fedora)
      mesaBuildDeps="See ${mesaDepsLog}..."
      Info "Fedora OSMesa Dependencies.[${mesaBuildDeps}]"
      Info "GLU Spec File.......[${mesaUtilsDir}/glu.spec]"
      Info
      $useSudo dnf builddep -y mesa >> $mesaDepsLog 2>&1
      Info "Fedora Mesa dependencies installed." && DisplayLogTail $mesaDepsLog 10
      $useSudo dnf builddep -y "${mesaUtilsDir}/glu.spec" >> $mesaDepsLog 2>&1
      Info "Fedora GLU dependencies installed." && DisplayLogTail $mesaDepsLog 5
      ;;
    arch)
      pkgbuildFile="${mesaUtilsDir}/PKGBUILD"
      mesaBuildDeps="$(echo `grep -wr 'depends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'` \
                            `grep -wr 'makedepends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'`)"
      Info "Arch Linux OSMesa Dependencies.[${mesaBuildDeps}]"
      Info "PKGBUILD File.......[$pkgbuildFile]"
      Info
      $useSudo pacman -S --noconfirm --needed $mesaBuildDeps >> $mesaDepsLog 2>&1
      Info "Arch Linux OSMesa and GLU dependencies installed."  && DisplayLogTail $mesaDepsLog 15
      ;;
    esac
  fi
  Info && Info "Build OSMesa and GLU static libraries..."
  chmod a+x "${mesaUtilsDir}/build_osmesa.sh"
  if [ "${OBS}" = "true" ]; then
    Info "Using sudo..........[No]"
    if [[ ("${platform_id}" = "redhat" && ${platform_ver} = 28) || \
          ("${platform_id}" = "debian" && ${platform_ver} = 10) || \
           "${platform_id}" = "arch" || -n "${LP3D_UCS_VER}" ]]; then
      osmesa_version=18.3.5
    else
      osmesa_version=17.2.6
    fi
    Info "Building OSMesa.....[${osmesa_version}]"
    env \
    OBS=${OBS} \
    RPM_BUILD=${RPM_BUILD} \
    LLVM_CONFIG=$(which llvm-config) \
    NO_GALLIUM=${no_gallium} \
    OSMESA_VERSION=${osmesa_version} \
    OSMESA_PREFIX=$WD/${DIST_DIR}/mesa/${platform_id} \
    ${mesaUtilsDir}/build_osmesa.sh &
    PID=$!
  else
    osmesa_version=17.2.6
    Info "Building OSMesa.....[${osmesa_version}]"
    Info "OSMesa Build Log....[${mesaBuildLog}]"
    Info
    env \
    OSMESA_VERSION=${osmesa_version} \
    OSMESA_PREFIX=$WD/${DIST_DIR}/mesa/${platform_id} \
    ${mesaUtilsDir}/build_osmesa.sh >> $mesaBuildLog 2>&1 &
    PID=$!
  fi

  TreatLongProcess $PID 60 "OSMesa and GLU build"

  local return_code=$?
  if [[ $return_code != 0 ]]; then
    OSMesaBuildAttempt=1
    return $return_code
  fi

  if [[ -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libOSMesa32.a" && \
        -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libGLU.a" ]]; then
    if [ "${OBS}" != "true" ]; then
      Info && Info "OSMesa and GLU build check..."
      DisplayCheckStatus "$mesaBuildLog" "Libraries have been installed in:" "1" "16"
      DisplayLogTail $mesaBuildLog 20
    fi
    return_code=0
    OSMesaBuildAttempt=1
  else
    if [ ! -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libOSMesa32.a" ]; then
      Info && Info "ERROR - libOSMesa32 not found. Binary was not successfully built."
    fi
    if [ ! -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libGLU.a" ]; then
      Info && Info "ERROR - libGLU not found. Binary was not successfully built."
    fi
    if [ "${OBS}" != "true" ]; then
      Info "------------------Build Log-------------------------"
      cat $mesaBuildLog
    fi
    return_code=1
  fi
  Info && Info "${1} library OSMesa build finished."
  return $return_code
}

# args: $1 = <log file>, $2 = <position>
DisplayLogTail() {
  if [[ -f "$1" && -s "$1" ]]; then
    logFile="$1"
    if [ "$2" = "" ]; then
      # default to 5 lines from the bottom of the file if not specified
      startPosition=-5
    elif [[ "${2:0:1}" != "-" || "${2:0:1}" != "+" ]]; then
      # default to the bottom of the file if not specified
      startPosition=-$2
    else
      startPosition=$2
    fi
    Info "Log file tail..."
    Info "$logFile last $startPosition lines:"
    tail $startPosition $logFile
  else
    Info "ERROR (log tail) - $1 not found or not valid!"
  fi
}

# args: $1 = <log file>, $2 = <search String>, $3 = <lines Before>, $4 = <lines After>
DisplayCheckStatus() {
  declare -i i; i=0
  for arg in "$@"; do
    i=i+1
    if test $i -eq 1; then s_buildLog="$arg"; fi
    if test $i -eq 2; then s_checkString="$arg"; fi
    if test $i -eq 3; then s_linesBefore="$arg"; fi
    if test $i -eq 4; then s_linesAfter="$arg"; fi
  done
  if [[ -f "$s_buildLog" && -s "$s_buildLog" ]]; then
    if test -z "$s_checkString"; then Info "ERROR - check string not specified."; return 1; fi
    if test -z "$s_linesBefore"; then s_linesBefore=2; Info "INFO - display 2 lines before"; fi
    if test -z "$s_linesAfter"; then s_linesAfter=10; Info "INFO - display 10 lines after"; fi
    Info "Checking if ${s_checkString} in ${s_buildLog}..."
    grep -B${s_linesBefore} -A${s_linesAfter} "${s_checkString}" $s_buildLog
  else
    Info "ERROR - Check display [$s_buildLog] not found or is not valid!"
  fi
}

# args: 1 = <start> (seconds mark)
ElapsedTime() {
  if test -z "$1"; then return 0; fi
  TIME_ELAPSED="$(((SECONDS - $1) % 60))sec"
  TIME_MINUTES=$((((SECONDS - $1) / 60) % 60))
  TIME_HOURS=$(((SECONDS - $1) / 3600))
  if [ "$TIME_MINUTES" -gt 0 ]; then
    TIME_ELAPSED="${TIME_MINUTES}mins $TIME_ELAPSED"
  fi
  if [ "$TIME_HOURS" -gt 0 ]; then
    TIME_ELAPSED="${TIME_HOURS}hrs $TIME_ELAPSED"
  fi
  echo "$TIME_ELAPSED"
}

# args: 1 = <pid>, 2 = <message interval>, [3 = <pretty label>]
TreatLongProcess() {
  declare -i i; i=0
  for arg in "$@"; do
    i=i+1
    if test $i -eq 1; then s_pid="$arg"; fi    # pid
    if test $i -eq 2; then s_msgint="$arg"; fi # message interval
    if test $i -eq 3; then s_plabel="$arg"; fi # pretty label
  done

  # initialize the duration counter
  s_start=$SECONDS

  # Validate the optional pretty label
  if test -z "$s_plabel"; then s_plabel="Create renderer"; fi

  # Spawn a process that coninually reports the command is running
  while Info "$(date): $s_plabel process $s_pid is running since `ElapsedTime $s_start`..."; \
  do sleep $s_msgint; done &
  s_nark=$!

  # Set a trap to kill the messenger when the process finishes
  trap 'kill $s_nark 2>/dev/null && wait $s_nark 2>/dev/null' RETURN

  # Wait for the process to finish and display exit code
  local s_return_code=0
  if wait $s_pid; then
    s_return_code=$?
    Info "$(date): $s_plabel process finished (returned ${s_return_code})"
  else
    s_return_code=$?
    Info "$(date): $s_plabel process terminated (returned ${s_return_code})"
  fi
  return $s_return_code
}

# args: 1 = <build folder>
InstallDependencies() {
  if [ "$OS_NAME" = "Linux" ]; then
    Msg="Install $1 build dependencies for target platform: [$platform_id]..."
    Info && Info $Msg && Info $Msg >> $depsLog 2>&1
    useSudo="sudo"
    Info "Using sudo..........[Yes]"
    case ${platform_id} in
    fedora|arch|ubuntu)
      true
      ;;
    *) 
      Msg="ERROR - Unable to process this target platform: [$platform_id]."
      Info $Msg && Info $Msg >> $depsLog 2>&1
      ;;
    esac
    if [ "$LP3D_BUILD_OS" = "appimage" ]; then
      depsLog=${LP3D_LOG_PATH}/${ME}_AppImage_deps_${1}.log
    else
      depsLog=${LP3D_LOG_PATH}/${ME}_${host}_deps_${1}.log
    fi
    Info "Platform_id.........[${platform_id}]"
    case ${platform_id} in
    fedora|centos)
      # Initialize install mesa
      case $1 in
      ldglite)
        specFile="$PWD/obs/ldglite.spec"
        ;;
      ldview)
        cp -f QT/LDView.spec QT/ldview-lp3d-qt5.spec
        specFile="$PWD/QT/ldview-lp3d-qt5.spec"
        sed -e 's/define qt5 0/define qt5 1/g' -e 's/kdebase-devel/make/g' -e 's/, kdelibs-devel//g' -i $specFile
        ;;
      povray)
        specFile="$PWD/unix/obs/povray.spec"
       ;;
      esac;
      rpmbuildDeps=$(rpmbuild --nobuild ${specFile} 2>&1 | grep 'needed by'| awk ' {print $1}')
      Info "Spec File...........[${specFile}]"
      Info "Dependencies List...[${rpmbuildDeps}]"
      if [[ -n "$build_osmesa" && "$OSMesaBuildAttempt" != 1 ]]; then
        BuildMesaLibs $1 $useSudo
      fi
      Info
      if [ "${platform_id}" = "fedora" ]; then
        $useSudo dnf builddep -y $specFile >> $depsLog 2>&1
      elif [ "${platform_id}" = "centos" ]; then
        $useSudo yum builddep -y $specFile >> $depsLog 2>&1
      fi
      Msg="${1} dependencies installed."
      Info $Msg && DisplayLogTail $depsLog 10 && Info $Msg >> $depsLog 2>&1
      ;;
    arch)
      case $1 in
      ldglite)
        pkgbuildFile="$PWD/obs/PKGBUILD"
        ;;
      ldview)
        cp -f QT/PKGBUILD QT/OBS/PKGBUILD
        pkgbuildFile="$PWD/QT/OBS/PKGBUILD"
        sed -e "s/pkgname=('ldview' 'ldview-osmesa')/pkgname=('ldview')/g" \
            -e "s/'mesa-libgl'/'mesa-libgl' 'tinyxml'/g" -i $pkgbuildFile
        if [ ! -d /usr/share/mime ]; then
          $useSudo mkdir /usr/share/mime
        fi
        ;;
      povray)
        pkgbuildFile="$PWD/unix/obs/PKGBUILD"
        ;;
      esac;
      pkgbuildDeps="$(echo `grep -wr 'depends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'` \
                           `grep -wr 'makedepends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'`)"
      Info "PKGBUILD File.......[${pkgbuildFile}]"
      Info "Dependencies List...[${pkgbuildDeps}]"
      Info
      $useSudo pacman -Syy --noconfirm --needed >> $depsLog 2>&1
      $useSudo pacman -Syu --noconfirm --needed >> $depsLog 2>&1
      $useSudo pacman -S --noconfirm --needed $pkgbuildDeps >> $depsLog 2>&1
      if [[ -n "$build_osmesa" && "$OSMesaBuildAttempt" != 1 ]]; then
        BuildMesaLibs $1 $useSudo
      fi
      Msg="${1} dependencies installed."
      Info $Msg && DisplayLogTail $depsLog 10 && Info $Msg >> $depsLog 2>&1
      ;;
    ubuntu)
      case $1 in
      ldglite)
        controlFile="$PWD/obs/debian/control"
        ;;
      ldview)
        controlFile="$PWD/QT/debian/control"
        sed -e '/^#Qt/d' -e '/libqt4-dev/d' -e '/qt6-base-dev/d' \
            -e 's/#Build-Depends/Build-Depends/g' -i $controlFile
        if [ "$LP3D_BUILD_OS" = "appimage" ]; then
          sed -e 's/ libkf5kio-dev,//g' \
              -e 's/ extra-cmake-modules,//g' \
              -e 's/ libkf5kdelibs4support5-bin,//g' -i $controlFile
        fi
        if [[ ("${DOCKER}" == "true" && "${GITHUB}" == "") || $platform_pretty == Snap* ]]; then
          sed 's/ kdelibs5-dev,//g' -i $controlFile
        fi
        ;;
      povray)
        controlFile="$PWD/unix/obs/debian/control"
        ;;
      esac;
      controlDeps=`grep Build-Depends $controlFile | cut -d: -f2| sed 's/(.*)//g' | tr -d ,`
      Info "Control File........[${controlFile}]"
      Info "Dependencies List...[${controlDeps}]"
      Info
      $useSudo apt-get update -qq >> $depsLog 2>&1
      $useSudo apt-get install -y $controlDeps >> $depsLog 2>&1
      Msg="${1} dependencies installed."
      Info $Msg && DisplayLogTail $depsLog 10 && Info $Msg >> $depsLog 2>&1
      ;;
      *)
      Msg="ERROR - Unknown platform [$platform_id]"
      Info $Msg && Info $Msg >> $depsLog 2>&1
      ;;
    esac;
  else
    Msg="ERROR - Platform is undefined or invalid [$OS_NAME] - Cannot continue."
    Info $Msg && Info $Msg >> $depsLog 2>&1
  fi
}

# args: <none>
ApplyLDViewStdlibHack() {
  Info "Apply stdlib error patch to LDViewGlobal.pri on $platform_pretty v$([ -n "$platform_ver" ] && [ "$platform_ver" != "undefined" ] && echo $platform_ver || true) ..."
  sed s/'    # detect system libraries paths'/'    # Suppress fatal error: stdlib.h: No such file or directory\n    QMAKE_CFLAGS_ISYSTEM = -I\n\n    # detect system libraries paths'/ -i LDViewGlobal.pri
  Info
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDGLite() {
  BUILD_CONFIG="CONFIG+=BUILD_CHECK CONFIG-=debug_and_release"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [[ -n "$build_osmesa" && "$get_local_libs" != 1 ]]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  if [ "$no_gallium" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=NO_GALLIUM"
  fi
  if [ "$get_local_libs" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_LOCAL=$LP3D_LL_USR"
  fi
  if [ "$local_freeglut" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_FREEGLUT_LOCAL"
  fi
  ${QMAKE_EXEC} -v && Info
  ${QMAKE_EXEC} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make -j${CPU_CORES}
    make install
  else
    make -j${CPU_CORES} > $2 2>&1
    make install >> $2 2>&1
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDView() {
  # Patch fatal error: stdlib.h: No such file or directory
  # on Docker, Fedora's platform_id is 'fedora', on OBS it is 'redhat'
  case ${platform_id} in
  redhat|centos|fedora|suse|mageia|openeuler)
     case ${platform_ver} in
     7|8|9|33|34|36|37|39|40|800|1500|1550|150000|150200|150300|2003|2103)
       ApplyLDViewStdlibHack
       ;;
     esac
    ;;
  arch)
    ApplyLDViewStdlibHack
    ;;
  debian|ubuntu)
    case ${platform_ver,} in
     7|8|9|10|11|12|18.04|20.04|22.04|24.04)
       ApplyLDViewStdlibHack
       ;;
    esac
    ;;
  esac
  BUILD_CONFIG="CONFIG+=BUILD_CHECK CONFIG-=debug_and_release"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  BUILD_CONFIG="$BUILD_CONFIG CONFIG+=BUILD_CUI_ONLY CONFIG+=USE_SYSTEM_LIBS"
  if [ "$prebuilt_3ds" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_3RD_PARTY_PREBUILT_3DS"
  fi
  if [[ -n "$build_osmesa" && "$get_local_libs" != 1 ]]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  if [ "$no_gallium" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=NO_GALLIUM"
  fi
  if [ "$build_tinyxml" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=BUILD_TINYXML"
  fi
  if [ "$build_gl2ps" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=BUILD_GL2PS"
  fi
  if [ "$get_local_libs" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_LOCAL=$LP3D_LL_USR"
  fi
  ${QMAKE_EXEC} -v && Info
  ${QMAKE_EXEC} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make -j1
    make install
  else
    make -j1 > $2 2>&1 &
    TreatLongProcess "$!" "60" "LDView make"
    make install >> $2 2>&1
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildPOVRay() {
  BUILD_CONFIG="--prefix=${DIST_PKG_DIR} LPUB3D_3RD_PARTY=yes --enable-watch-cursor"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG --enable-debug"
  fi
  if [ "$build_sdl2" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG --with-libsdl2=from-src"
  else
    BUILD_CONFIG="$BUILD_CONFIG --with-libsdl2"
  fi
  if [ "$OBS_RPM1315_BUILD_OPTS" = 1 ]; then
    BUILD_CONFIG="$OBS_RPM_BUILD_CONFIG $BUILD_CONFIG"
    BUILD_FLAGS="$BUILD_FLAGS CXXFLAGS=\"$OBS_RPM_BUILD_CXXFLAGS\" CFLAGS=\"$OBS_RPM_BUILD_CFLAGS\""
  fi
  if [ "$MACOS_POVRAY_NO_OPTIMIZ" = "true" ]; then
    BUILD_CONFIG="$BUILD_CONFIG --without-optimiz"
  fi
  if [ "$get_local_libs" = 1 ]; then
    [ -z "$BUILD_FLAGS" ] && \
    BUILD_FLAGS="CPPFLAGS=\"-I$LP3D_LL_USR/include\" LDFLAGS=\"$LP3D_LDFLAGS\"" || \
    BUILD_FLAGS="$BUILD_FLAGS CPPFLAGS=\"-I$LP3D_LL_USR/include\" LDFLAGS=\"$LP3D_LDFLAGS\""
    export PKG_CONFIG_PATH=$LP3D_LL_USR/lib64/pkgconfig:$PKG_CONFIG_PATH && \
    Info "Prepend PKG_CONFIG_PATH for POVRay build: $PKG_CONFIG_PATH"
  fi
  #Info "DEBUG_DEBUG BUILD_FLAGS: $BUILD_FLAGS"
  #Info "DEBUG_DEBUG PRINT_ENV:   $BUILD_CONFIG"
  [ ! "$OS_NAME" = "Darwin" ] && export POV_IGNORE_SYSCONF_MSG="yes" || true
  chmod a+x unix/prebuild3rdparty.sh && ./unix/prebuild3rdparty.sh
  [ -n "$BUILD_FLAGS" ] && \
  Info && Info "BUILD_FLAGS: $BUILD_FLAGS" && \
  Info "BUILD_CONFIG: $BUILD_CONFIG" && \
  env $BUILD_FLAGS ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." $BUILD_CONFIG || \
  Info && Info "BUILD_CONFIG: $BUILD_CONFIG" && \
  ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." $BUILD_CONFIG
  #Info "DEBUG_DEBUG CONFIG.LOG: " && cat config.log
  if [ "${OBS}" = "true" ]; then
    make -j${CPU_CORES}
    make install
    make check
    if [ ! -f "unix/lpub3d_trace_cui" ]; then
      Info "ERROR - lpub3d_trace_cui build failed!"
      Info "The config.log may give some insights."
      cat config.log
    fi
  else
    make -j${CPU_CORES} > $2 2>&1 &
    TreatLongProcess "$!" "60" "POV-Ray make"
    make check >> $2 2>&1
    make install >> $2 2>&1
  fi
}

# Package the renderers
canPackageRenderers="true"
function package_renderers()
{
    if [[ "${OBS}" = "true" || "${SNAP}" = "true" ]]; then
        Info "Cannot create renderer package under OBS or SNAP builds"
        return
    fi
    if [ -d "/out" ]; then 
        LP3D_OUT_PATH=/out
    elif [ -d "/buildpkg" ]; then 
        LP3D_OUT_PATH=/buildpkg
    else
        LP3D_OUT_PATH=${LP3D_LOG_PATH}
    fi
	declare -r p=Package
    LP3D_ARCH=${TARGET_CPU}
    LP3D_BASE=${platform_id}-${platform_ver}
    LP3D_RNDR_VERSION=${LP3D_VERSION}.${LP3D_VER_REVISION}.${LP3D_VER_BUILD}
    LP3D_RENDERERS=LPub3D-${LP3D_RNDR_VERSION}-renderers-${LP3D_BASE}-${LP3D_ARCH}.tar.gz
    echo -n "-Create renderer package ${LP3D_OUT_PATH}/${LP3D_RENDERERS}..."
    ( cd "${DIST_PKG_DIR}/" || return && \
    tar -czf "${LP3D_RENDERERS}"  \
    "--exclude=${VER_LDVIEW}/lib" \
    "--exclude=${VER_LDVIEW}/include" \
    "--exclude=${VER_LDVIEW}/bin/*.exp" \
    "--exclude=${VER_LDVIEW}/bin/*.lib" \
    "--exclude=${VER_LDVIEW}/resources/*Messages.ini" \
    "${VER_LDGLITE}/" "${VER_LDVIEW}/" "${VER_POVRAY}/" && \
    sha512sum "${LP3D_RENDERERS}" > "${LP3D_RENDERERS}.sha512" && \
    mv -f "${LP3D_RENDERERS}" "${LP3D_RENDERERS}.sha512" \
    "${LP3D_OUT_PATH}/" ) >$p.out 2>&1 && rm $p.out
    [ -f $p.out ] && echo "ERROR" && tail -80 $p.out || echo "Ok"
    Info
}

# **************** Begin Main Script *****************************

# Grab the script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# Start message and set sourced flag
if [ "${ME}" = "CreateRenderers.sh" ]; then
  SOURCED="false"
  Info && Info "Start $ME execution at $PWD..."
else
  SOURCED="true"
  Info && Info "Start CreateRenderers execution at $PWD..."
fi
if [[ "${SOURCED}" = "false" && -f "rendererVars.sh" ]]; then
  # Info && cat rendererVars.sh
  source rendererVars.sh && importedRendererVars=1
fi

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
  parent_dir=${PWD##*/}
  if  [ "$parent_dir" = "utilities" ]; then
    if [ "$OS_NAME" = "Darwin" ]; then
      chkdir="$(cd /../../../ && echo $PWD)"
    elif [ -n "$GITHUB_JOB" ]; then
      chkdir="$(realpath ../../)"
    else
      chkdir="$(realpath ../../../)"
    fi
    if [[ -d "$chkdir" && -f "$chkdir/LPub3D.pro" ]]; then
      WD=$chkdir
    else
      Info "ERROR - 'WD' environment varialbe not specified. Usage: env WD=... bash $0"
      exit 2
    fi
  else
    WD=$PWD
  fi
  Info "WARNING - 'WD' environment varialbe not specified. Using $WD"
fi

# Grab the calling dir
CallDir=$PWD

# Tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# Populate the OS Name
OS_NAME=$(uname)

# Get CPU arch - 'uname -m' returns x86_64, armv7l or aarch64
TARGET_CPU=$(uname -m)

# Define build architecture and cached renderer paths
if [[ "$TARGET_CPU" = "x86_64" || "$TARGET_CPU" = "aarch64" || "$TARGET_CPU" = "arm64" ]]; then
  buildArch="64bit_release"
else
  buildArch="32bit_release"
fi

# QMake CPU value for ARM 64bit is arm64
TARGET_CPU_QMAKE=${TARGET_CPU}
if [ "${TARGET_CPU}" = "aarch64" ]; then
  TARGET_CPU_QMAKE="arm64"
fi

# Initialize OBS if not in command line input - FlatPak does not call this script
if [[ "${OBS}" = "" && "${DOCKER}" = "" && "${CI}" = "" && "${SNAP}" = "" ]]; then
  OBS=true
fi

Info && Info "Building.................[LPub3D 3rd Party Renderers]"
[ -n "$LPUB3D" ] && Info "LPub3D Build Folder......[$LPUB3D]" || :
[ -n "$importedRendererVars" ] && Info "Renderer Build Variables.[rendererVars.sh]" || :

# Get pretty platform name, short platform name and platform version
if [ "$OS_NAME" = "Darwin" ]; then
  platform_pretty=$(echo `sw_vers -productName` `sw_vers -productVersion`)
  platform_id=macos
  platform_ver=$(echo `sw_vers -productVersion`)
else
  platform_id=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $OS_NAME | awk '{print tolower($0)}') #'
  platform_pretty=$(. /etc/os-release 2>/dev/null; [ -n "$PRETTY_NAME" ] && echo "$PRETTY_NAME" || echo $OS_NAME)
  platform_ver=$(. /etc/os-release 2>/dev/null; [ -n "$VERSION_ID" ] && echo $VERSION_ID || echo 'undefined')
  if [ "${OBS}" = "true" ]; then
    if [ "$RPM_BUILD" = "true" ]; then
      Info "OBS Build Family.........[RPM_BUILD]"
      if [ -n "$TARGET_VENDOR" ]; then
        platform_id=$(echo $TARGET_VENDOR | awk '{print tolower($0)}') #'
      else
        Info "WARNING - Open Build Service did not provide a target platform."
        platform_id=$(echo $OS_NAME | awk '{print tolower($0)}') #'
      fi
      if [ -n "$PLATFORM_PRETTY_OBS" ]; then
        platform_pretty=$PLATFORM_PRETTY_OBS
        [ "$platform_id" = "suse" ] && export PRETTY_NAME=$PLATFORM_PRETTY_OBS || true
      else
        Info "WARNING - Open Build Service did not provide a platform pretty name."
        platform_pretty=$OS_NAME
      fi
      if [ -n "$PLATFORM_VER_OBS" ]; then
        platform_ver=$PLATFORM_VER_OBS
      else
        Info "WARNING - Open Build Service did not provide a platform version."
        platform_ver=undefined
      fi
    elif [ -n "${LP3D_UCS_VER}" ]; then
      platform_pretty="Univention Corporate Server"
      platform_ver="${LP3D_UCS_VER}"
    fi
  fi
  # change Arch Pretty Name export Arch Extra codes
  if [ "$platform_id" = "arch" ]; then
    platform_pretty=$PLATFORM_PRETTY
    platform_ver=$PLATFORM_VER
  fi
fi
[ -n "$platform_id" ] && host=$platform_id || host=undefined

# Display platform settings
Info "Build Working Directory..[${CallDir}]"
[ -n "${LP3D_UCS_VER}" ] && \
Info "Platform ID..............[ucs]" || \
Info "Platform ID..............[${platform_id}]"
if [ "$LP3D_BUILD_OS" = "snap" ]; then
  platform_pretty="Snap (using $platform_pretty)"
  SNAP="true"
elif [ "$LP3D_BUILD_OS" = "appimage" ]; then
  platform_pretty="AppImage (using $platform_pretty)"
fi
if [ "${DOCKER}" = "true" ]; then
  Info "Platform Pretty Name.....[Docker Container - ${platform_pretty}]"
  if [ "${platform_id}" = "arch" ]; then
     [ -n "$build_osmesa" ] && Info "OSMesa...................[Build from source]"
  fi
elif [ "${CI}" = "true" ]; then
  Info "Platform Pretty Name.....[CI - ${platform_pretty}]"
elif [ "${OBS}" = "true" ]; then
  if [[ "${TARGET_CPU}" = "aarch64" || "${TARGET_CPU}" = "arm7l" ]]; then
    platform_pretty="$platform_pretty (ARM-${TARGET_CPU})"
  fi
  Info "Platform Pretty Name.....[Open Build Service - ${platform_pretty}]"
  [ "$platform_id" = "arch" ] && build_tinyxml=1 || true
  [ -n "$get_qt5" ] && Info "Get Qt5 library..........[$LP3D_QT5_BIN]" || true
  [[ -n "$build_osmesa" && ! -n "$get_local_libs" ]] && Info "OSMesa...................[Build from source]"
  [ -n "$no_gallium" ] && Info "Gallium driver...........[Not available]" || true
  [ -n "$get_local_libs" ] && Info "Get local libraries......[Using OSMesa, LLVM, OpenEXR, and DRM from $LP3D_LL_USR/lib64]" || true
  [ -n "$build_sdl2" ] && Info "SDL2.....................[Build from source]" || true
  [ -n "$build_tinyxml" ] && Info "TinyXML..................[Build from source]" || true
  [ -n "$build_gl2ps" ] && Info "GL2PS....................[Build from source]" || true
  [ -n "$prebuilt_3ds" ] && Info "3DS......................[Use pre-built library]" || \
                            Info "3DS......................[Build from source]"
else
  Info "Platform Pretty Name.....[${platform_pretty}]"
fi

Info "Platform Version.........[$platform_ver]"
Info "Target CPU...............[${TARGET_CPU}]"
Info "Dist Working Directory...[$WD]"

# Distribution directory
DIST_DIR=${LP3D_3RD_DIST_DIR:-}
if [ -z "$DIST_DIR" ]; then
  if [ "$OS_NAME" = "Darwin" ]; then
    DIST_DIR=lpub3d_macos_3rdparty
  else
    DIST_DIR=lpub3d_linux_3rdparty
  fi
fi
DIST_PKG_DIR=${WD}/${DIST_DIR}
if [ ! -d "${DIST_PKG_DIR}" ]; then
  mkdir -p ${DIST_PKG_DIR}
fi
export DIST_PKG_DIR="${DIST_PKG_DIR}"

Info "Dist Directory...........[${DIST_PKG_DIR}]"

# Change to Working directory
# Travis: /home/travis/build/trevorsandy
cd ${WD}

# set log output path
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=${LP3D_LOG_PATH:-$WD} || :
Info "Log Path.................[${LP3D_LOG_PATH}]"

# expose GitHub Actions variables
if [[ -n "$CD" || -n "${GITHUB}" ]]; then
    Info
    [ -n "$CI" ] && Info "CI.......................${CI}" || :
    [ -n "$OBS" ] && Info "OBS......................${OBS}" || :
    [ -n "$DOCKER" ] && Info "DOCKER...................${DOCKER}" || :
    [ -n "$GITHUB" ] && Info "GITHUB...................${GITHUB}" || :
    [ -n "$LP3D_CPU_CORES" ] && Info "CPU CORES................${LP3D_CPU_CORES}" || :
    [ -z "$LP3D_NO_DEPS" ] && Info "LOAD LIB DEPENDENCY......true" || :
    [ -n "$LP3D_NO_CLEANUP" ] && Info "NO CLEANUP...............${LP3D_NO_CLEANUP}" || :
    [ -n "$LP3D_BASE" ] && Info "BUILD BASE...............${LP3D_BASE}" || :
    [ -n "$LP3D_ARCH" ] && Info "BUILD ARCH...............${LP3D_ARCH}" || :
    [ -n "$LP3D_APPIMAGE" ] && Info "APPIMAGE.................${LP3D_APPIMAGE}" || :
fi

# Setup LDraw Library - for testing LDView and LDGLite and also used by LPub3D test
if [ -z "$LDRAWDIR_ROOT" ]; then
  if [ "$OS_NAME" = "Darwin" ]; then
    LDRAWDIR_ROOT=${HOME}/Library
  else
    LDRAWDIR_ROOT=${HOME}
  fi
fi
if [ -z "$LDRAWDIR" ]; then
  LDRAWDIR=${LDRAWDIR_ROOT}/ldraw
  export LDRAWDIR=${LDRAWDIR}
fi

if [ "$OBS" != "true" ]; then
  if [ ! -f "${DIST_PKG_DIR}/complete.zip" ]; then
    Info && Info "LDraw archive complete.zip not found at ${DIST_PKG_DIR}."
    if [ ! -f "complete.zip" ]; then
      Info "Downloading complete.zip..." && \
      curl $curlopts https://library.ldraw.org/library/updates/complete.zip -o ${DIST_PKG_DIR}/complete.zip
    else
      ldrawlib=$(echo $PWD/complete.zip)
      Info "Linking complete.zip..." && (cd ${DIST_PKG_DIR} && ln -sf ${ldrawlib} complete.zip)
    fi
  fi
  if [ ! -f "${DIST_PKG_DIR}/lpub3dldrawunf.zip" ]; then
    Info "LDraw archive lpub3dldrawunf.zip not found at ${DIST_PKG_DIR}."
    if [ ! -f "lpub3dldrawunf.zip" ]; then
      Info "Downloading lpub3dldrawunf.zip..." && \
      curl $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/lpub3dldrawunf.zip -o ${DIST_PKG_DIR}/lpub3dldrawunf.zip
    else
      ldrawlib=$(echo $PWD/lpub3dldrawunf.zip)
      Info "Linking lpub3dldrawunf.zip..." && (cd ${DIST_PKG_DIR} && ln -sf ${ldrawlib} lpub3dldrawunf.zip)
    fi
  fi
fi
if [ ! -d "${LDRAWDIR}/parts" ]; then
  if [ "$OBS" != "true" ]; then
    [ ! -f "complete.zip" ] && \
    cp -f ${DIST_PKG_DIR}/complete.zip . || :
  fi
  Info && Info "Extracting LDraw library into ${LDRAWDIR}..."
  unzip -od ${LDRAWDIR_ROOT} -q complete.zip;
  if [ -d "${LDRAWDIR}/parts" ]; then
    Info "LDraw library extracted. LDRAWDIR defined."
  fi
elif [ "$OS_NAME" != "Darwin" ]; then
  Info "LDraw Library............[${LDRAWDIR}]"
fi
# QMake and additional LDraw configuration
if [ "$OS_NAME" = "Darwin" ]; then
  Info "LDraw Library............[${LDRAWDIR}]"
  Info && Info "set LDRAWDIR in environment.plist..."
  chmod +x ${LPUB3D}/builds/utilities/set-ldrawdir.command && ./${LPUB3D}/builds/utilities/set-ldrawdir.command
  grep -A1 -e 'LDRAWDIR' ~/.MacOSX/environment.plist
  Info "set LDRAWDIR Completed."

  # Qt setup - MacOS
  QMAKE_EXEC=qmake
else
  # Qt setup - Linux
  if [ -f $LP3D_QT5_BIN/qmake ] ; then
    QMAKE_EXEC=$LP3D_QT5_BIN/qmake
  else
    export QT_SELECT=qt5
    if [ -x /usr/bin/qmake-qt5 ] ; then
      QMAKE_EXEC=/usr/bin/qmake-qt5
    else
      QMAKE_EXEC=qmake
    fi
  fi
  # set dependency profiler and nubmer of CPUs
  LDD_EXEC=ldd
  if [[ "$TARGET_CPU" = "aarch64" || "$TARGET_CPU" = "arm64" ]]; then
    CPU_CORES=1
  else
    if [ -n "${LP3D_CPU_CORES}" ]; then
      CPU_CORES=${LP3D_CPU_CORES}
    else
      CPU_CORES=$(nproc)
    fi
  fi
fi

# get Qt version
Info && ${QMAKE_EXEC} -v && Info
QMAKE_EXEC="${QMAKE_EXEC} -makefile"

# backup ld_library_path
LP3D_LD_LIBRARY_PATH_SAVED=$LD_LIBRARY_PATH

# initialize mesa build flag
OSMesaBuildAttempt=0

# processor and linkier flags for building local libs
if [ "$get_local_libs" = 1 ]; then
  export PATH=$LP3D_LL_USR/bin:$PATH && \
  Info "Prepend PATH with: $PATH"
  export LD_LIBRARY_PATH=$LP3D_LL_USR/bin:$LP3D_LL_USR/lib64:$LD_LIBRARY_PATH && \
  Info "Prepend LD_LIBRARY_PATH with: $LD_LIBRARY_PATH"
  LP3D_LDFLAGS="-L$LP3D_LL_USR/lib64" && \
  Info "Set LP3D_LDFLAGS to $LP3D_LDFLAGS" && Info
  export Q_LDFLAGS="$LP3D_LDFLAGS"
fi

# renderer versions
VER_LDGLITE=ldglite-1.3
VER_LDVIEW=ldview-4.5
VER_POVRAY=lpub3d_trace_cui-3.8
# renderer paths
LP3D_LDGLITE=${DIST_PKG_DIR}/${VER_LDGLITE}/bin/${TARGET_CPU_QMAKE}/ldglite
LP3D_LDVIEW=${DIST_PKG_DIR}/${VER_LDVIEW}/bin/${TARGET_CPU_QMAKE}/ldview
LP3D_POVRAY=${DIST_PKG_DIR}/${VER_POVRAY}/bin/${TARGET_CPU}/lpub3d_trace_cui

# install build dependencies for MacOS
if [ "$OS_NAME" = "Darwin" ]; then
  Msg="Install $OS_NAME renderer build dependencies..."
  Info && Info $Msg && Info $Msg > $depsLog 2>&1
  Info "----------------------------------------------------"
  Info "Platform.................[macos]"
  Info "Using sudo...............[No]"
  for buildDir in ldview povray; do
     case ${buildDir} in
     ldview)
       brewDeps="tinyxml gl2ps libjpeg minizip"
       ;;
     povray)
       brewDeps="$brewDeps boost openexr sdl2 libtiff autoconf automake"
       if [ "${CI}" != "true" ]; then
         brewDeps="$brewDeps pkg-config"
       fi
       ;;
     esac
  done
  Info "Checking for X11 (xquartz) at /usr/X11..."
  if [[ -d /usr/X11/lib && /usr/X11/include ]]; then
    Info "Good to go - X11 found."
  else
    Msg="ERROR - Sorry to say friend, I cannot go on - X11 not found."
    Info $Msg && Info $Msg > $depsLog 2>&1
    if [ "${CI}" != "true" ]; then
      Info "  You can install xquartz using homebrew:"
      Info "  \$ brew cask list"
      Info "  \$ brew cask install xquartz"
      Info "  Note: elevated access password will be required."
      Info "  Build will terminate."
    fi
    exit 1
  fi
  depsLog=${LP3D_LOG_PATH}/${ME}_${host}_deps_$OS_NAME.log
  if [ -n "$brewDeps" ]; then
    Info "Dependencies List........[X11 boost ${brewDeps}]"
    if [ "${CI}" = "true" ]; then
     Info  "--- Skipped brew update to save time"
    else
     Info && Info "Enter u to update brew or any key to skip brew update."
     read -n 1 -p "Do you want to update brew? : " getoption
     if [ "$getoption" = "u" ] || [ "$getoption" = "U" ]; then
       Info "--- Updating brew - this may take a while..."
       brew update >> $depsLog 2>&1
     fi
    fi
    Info  "--- Install depenencies..."
    brew install $brewDeps >> $depsLog 2>&1
    Msg="$OS_NAME dependencies installed."
    Info $Msg && DisplayLogTail $depsLog 10 && Info $Msg >> $depsLog 2>&1
  else
    Msg="Renderer artefacts exist, nothing to build. Install dependencies skipped"
    Info $Msg && DisplayLogTail $depsLog 3 && Info $Msg >> $depsLog 2>&1
  fi
  # Set povray --without-optimiz flag on macOS High Sierra 10.13
  [ "$(echo $platform_ver | cut -d. -f2)" = 13 ] && MACOS_POVRAY_NO_OPTIMIZ="true" || true
  # set dependency profiler and nubmer of CPUs
  LDD_EXEC="otool -L"
  CPU_CORES=$(sysctl -n hw.ncpu)
fi

# List 'LP3D_*' environment variables
Info && Info "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do echo $line=${!line}; done
Info

# =======================================
# Main loop
# =======================================
for buildDir in ldglite ldview povray; do
  buildDirUpper="$(echo ${buildDir} | awk '{print toupper($0)}')"
  artefactVer="VER_${buildDirUpper}"
  artefactBinary="LP3D_${buildDirUpper}"
  buildLog=${LP3D_LOG_PATH}/${ME}_${host}_build_${buildDir}.log
  linesBefore=1
  case ${buildDir} in
  ldglite)
    curlCommand="https://github.com/trevorsandy/ldglite/archive/master.tar.gz"
    checkString="LDGLite Output"
    linesAfter="2"
    buildCommand="BuildLDGLite"
    validSubDir="app"
    validExe="${validSubDir}/${buildArch}/ldglite"
    buildType="release"
    displayLogLines=10
    ;;
  ldview)
    curlCommand="https://github.com/trevorsandy/ldview/archive/lpub3d-build.tar.gz"
    checkString="LDView Image Output"
    linesAfter="9"
    buildCommand="BuildLDView"
    validSubDir="OSMesa"
    validExe="${validSubDir}/${buildArch}/ldview"
    buildType="release"
    displayLogLines=100
    ;;
  povray)
    curlCommand="https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.tar.gz"
    checkString="Render Statistics"
    linesAfter="42"
    buildCommand="BuildPOVRay"
    validSubDir="unix"
    validExe="${validSubDir}/lpub3d_trace_cui"
    buildType="release"
    displayLogLines=10
    ;;
  esac

  if [ "$OBS" = "true" ]; then
    # OBS build setup routine...
    if [ -f "${buildDir}.tar.gz" ]; then
      ExtractArchive ${buildDir} ${validSubDir}
    else
      Msg="ERROR - Unable to find ${buildDir}.tar.gz at $PWD"
      Info && Info $Msg && Info $Msg >> $buildLog 2>&1
    fi
    if [[ -n "$build_osmesa" && -z "$get_local_libs" && "$OSMesaBuildAttempt" != 1 ]]; then

      BuildMesaLibs

      if [[ $? != 0 ]]; then
        OSMesaBuildAttempt=1
        Msg="Build OSMesa failed with return code $?. $ME will terminate."
        Info && Info $Msg && Info $Msg >> $buildLog 2>&1
        exit 1
      fi
    fi
    # Building POVRay on openSUSE Leap 42.1-3 or SLE-12 (old builds that may not longer be being built)
    if [[ "$platform_id" = "suse" && "${buildDir}" = "povray" && $(echo "$platform_ver" | grep -E '1315') ]]; then
      OBS_RPM_BUILD_CFLAGS="$RPM_OPTFLAGS -fno-strict-aliasing -Wno-multichar"
      OBS_RPM_BUILD_CXXFLAGS="$OBS_RPM_BUILD_CFLAGS -std=c++11 -Wno-reorder -Wno-sign-compare -Wno-unused-variable \
      -Wno-unused-function -Wno-comment -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-switch"
      OBS_RPM_BUILD_CONFIG="--disable-dependency-tracking --disable-strip --disable-optimiz --with-boost-libdir=${RPM_LIBDIR}"
      Info && Info "Using RPM_BUILD_FLAGS: $OBS_RPM_BUILD_CXXFLAGS and  OBS_RPM_BUILD_CONFIG: ${OBS_RPM_BUILD_CONFIG}" && Info
      OBS_RPM1315_BUILD_OPTS=1
    fi
  else
    # Check if build folder exist - donwload tarball and extract even if binary exists (to generate dependency lists)
    Info && Info "Setup ${!artefactVer} source files..."
    Info "----------------------------------------------------"
    if [ ! -d "${buildDir}/${validSubDir}" ]; then
      # Check if build dependencies or no binary...
      if [[ ! -f "${!artefactBinary}" || ! "$LP3D_NO_DEPS" = "true" ]]; then
        Info && Info "$(echo ${buildDir} | awk '{print toupper($0)}') build folder does not exist. Checking for tarball archive..."
        if [ ! -f ${buildDir}.tar.gz ]; then
          Info "$(echo ${buildDir} | awk '{print toupper($0)}') tarball ${buildDir}.tar.gz does not exist. Downloading..."
          curl $curlopts ${curlCommand} -o ${buildDir}.tar.gz
        fi
        ExtractArchive ${buildDir} ${validSubDir}
      fi
    else
      cd ${buildDir}
    fi
    # Install build dependencies - even if binary exists...
    if [[ ! "$OS_NAME" = "Darwin" && ! "$OBS" = "true" && ! "$LP3D_NO_DEPS" = "true" ]]; then
      Info && Info "Install ${!artefactVer} build dependencies..."
      Info "----------------------------------------------------"
      InstallDependencies ${buildDir}
      sleep .5
    fi
  fi

  # Perform build - only if binary does not exist
  Info && Info "Build ${!artefactVer}..."
  Info "----------------------------------------------------"
  if [ ! -f "${!artefactBinary}" ]; then
    ${buildCommand} ${buildType} ${buildLog}
    [ -f "${validExe}" ] && Info && Info "$LDD_EXEC check ${buildDir}..." && \
    $LDD_EXEC ${validExe} 2>/dev/null || Info "ERROR - $LDD_EXEC ${validExe} failed."
    if [ "${OBS}" != "true" ]; then
      if [ -f "${validExe}" ]; then
        Info && Info "Build check - ${buildDir}..."
        DisplayCheckStatus "${buildLog}" "${checkString}" "${linesBefore}" "${linesAfter}" && Info
        DisplayLogTail ${buildLog} ${displayLogLines}
      else
        Msg="ERROR - ${validExe} not found. Binary was not successfully built"
        Info && Info $Msg && Info $Msg >> $buildLog 2>&1
        Info "------------------Build Log-------------------------"
        cat ${buildLog}
        Info "----------------End-Build Log-----------------------"
        canPackageRenderers="false"
      fi
    fi
    Msg="Build ${buildDir} finished."
    Info && Info $Msg && Info && Info $Msg >> $buildLog 2>&1
  else
    Msg="Renderer artefact binary for ${!artefactVer} exists - build skipped."
    Info && Info $Msg && Info && Info $Msg >> $buildLog 2>&1
  fi
  cd ${WD}
done
# Package renderers as a deliverable
if [[ "${canPackageRenderers}" = "true"  ]]; then
  Info "Package renderers for download disabled."
  #package_renderers
else
  Info "Cannot package renderers."
fi
# Restore ld_library_path
export LD_LIBRARY_PATH=$LP3D_LD_LIBRARY_PATH_SAVED

exit 0
