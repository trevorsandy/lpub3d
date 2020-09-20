#!/bin/bash
#
# Build all LPub3D 3rd-party renderers
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update Oct 09, 2019
#  Copyright (c) 2017 - 2020 by Trevor SANDY
#

# sample commands [call from root build directory - e.g. lpub3d]:
# -export WD=$PWD; export DOCKER=true; chmod +x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# -export OBS=false; source ${SOURCE_DIR}/builds/utilities/CreateRenderers.sh

# NOTE: OBS flag is 'ON' by default, if not using DOCKER, be sure to set this flag in your build command accordingly
# NOTE: elevated access required for dnf builddeps, execute with sudo if running noninteractive

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

# Functions
Info () {
  if [ "${SOURCED}" = "true" ]
  then
    echo "   renderers: ${*}" >&2
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
    rm -rf $1.tar.gz && Info "Cleanup archive $1.tar.gz."
    cd $1
  else
    Info "ERROR - $1.tar.gz did not extract properly."
  fi
}

BuildMesaLibs() {
  mesaUtilsDir="$CallDir/builds/utilities/mesa"
  if [ ! "${OBS}" = "true" ]; then
    mesaDepsLog=${LOG_PATH}/${ME}_${host}_mesadeps_${1}.log
    mesaBuildLog=${LOG_PATH}/${ME}_${host}_mesabuild_${1}.log
  fi
  if [ -z "$2" ]; then
    useSudo=
  else
    useSudo=$2
  fi

  Info "Update OSMesa.......[Yes]"
  if [ ! "${OBS}" = "true" ]; then
    case ${platform_id} in
    fedora)
      mesaBuildDeps="See ${mesaDepsLog}..."
      Info "Fedora OSMesa Dependencies.[${mesaBuildDeps}]"
      Info "GLU Spec File.......[${mesaUtilsDir}/glu.spec]"
      Info
      $useSudo dnf builddep -y mesa > $mesaDepsLog 2>&1
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
      $useSudo pacman -S --noconfirm --needed $mesaBuildDeps > $mesaDepsLog 2>&1
      Info "Arch Linux OSMesa and GLU dependencies installed."  && DisplayLogTail $mesaDepsLog 15
      ;;
    esac
  fi
  Info && Info "Build OSMesa and GLU static libraries..."
  chmod a+x "${mesaUtilsDir}/build_osmesa.sh"
  if [ "${OBS}" = "true" ]; then
    Info "Using sudo..........[No]"
    if [[ ("${platform_id}" = "redhat" && ${platform_ver} = 28) || ("${platform_id}" = "arch") ]]; then
      osmesa_version=18.3.5
    else
      osmesa_version=17.2.6
    fi
    Info "Building OSMesa.....[${osmesa_version}]"
    env \
    NO_GALLIUM=${no_gallium} \
    OSMESA_VERSION=${osmesa_version} \
    OSMESA_PREFIX=$WD/${DIST_DIR}/mesa/${platform_id} \
    ${mesaUtilsDir}/build_osmesa.sh &
  else
    #if [[ ("${platform_id}" = "fedora" && ${platform_ver} = 26) || "${platform_id}" = "arch" ]]; then
    if [ "${platform_id}" = "arch" ]; then
      osmesa_version=18.3.5
    else
      osmesa_version=17.2.6
    fi
    Info "Building OSMesa.....[${osmesa_version}]"
    Info "OSMesa Build Log....[${mesaBuildLog}]"
    env \
    OSMESA_VERSION=${osmesa_version} \
    OSMESA_PREFIX=$WD/${DIST_DIR}/mesa/${platform_id} \
    ${mesaUtilsDir}/build_osmesa.sh > $mesaBuildLog 2>&1 &
  fi

  TreatLongProcess $! 60 "OSMesa and GLU build"

  if [[ -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libOSMesa32.a" && \
        -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libGLU.a" ]]; then
    if [ ! "${OBS}" = "true" ]; then
      Info &&  Info "OSMesa and GLU build check..."
      DisplayCheckStatus "$mesaBuildLog" "Libraries have been installed in:" "1" "16"
      DisplayLogTail $mesaBuildLog 20
    fi
    OSMesaBuilt=1
  else
    if [ ! -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libOSMesa32.a" ]; then
      Info && Info "ERROR - libOSMesa32 not found. Binary was not successfully built."
    fi
    if [ ! -f "$WD/${DIST_DIR}/mesa/${platform_id}/lib/libGLU.a" ]; then
      Info && Info "ERROR - libGLU not found. Binary was not successfully built."
    fi
    if [ ! "${OBS}" = "true" ]; then
      Info "------------------Build Log-------------------------"
      cat $mesaBuildLog
    fi
  fi
  Info && Info "${1} library OSMesa build finished."
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
  if wait $s_pid; then
    Info "$(date): $s_plabel process finished (returned $?)"
  else
    Info "$(date): $s_plabel process terminated (returned $?)"
  fi
}

# args: 1 = <build folder>
InstallDependencies() {
  if [ "$OS_NAME" = "Linux" ]; then
    Info &&  Info "Install $1 build dependencies..."
    useSudo="sudo"
    Info "Using sudo..........[Yes]"
    case ${platform_id} in
    fedora|arch|ubuntu)
      true
      ;;
    *)
      Info "ERROR - Unable to process this target platform: [$platform_id]."
      ;;
    esac
    if [ "$LP3D_BUILD_APPIMAGE" = "true" ]; then
      depsLog=${LOG_PATH}/${ME}_AppImage_deps_${1}.log
    else
      depsLog=${LOG_PATH}/${ME}_${host}_deps_${1}.log
    fi
    Info "Platform_id.........[${platform_id}]"
    case ${platform_id} in
    fedora)
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
      rpmbuildDeps="See $depsLog..."
      Info "Spec File...........[${specFile}]"
      Info "Dependencies List...[${rpmbuildDeps}]"
      if [[ -n "$build_osmesa" && ! "$OSMesaBuilt" = 1 ]]; then
        BuildMesaLibs $1 $useSudo
      fi
      Info
      $useSudo dnf builddep -y $specFile > $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
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
      $useSudo pacman -Syy --noconfirm --needed > $depsLog 2>&1
      $useSudo pacman -Syu --noconfirm --needed >> $depsLog 2>&1
      $useSudo pacman -S --noconfirm --needed $pkgbuildDeps >> $depsLog 2>&1
      if [[ -n "$build_osmesa" && ! "$OSMesaBuilt" = 1 ]]; then
        BuildMesaLibs $1 $useSudo
      fi
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
    ubuntu)
      case $1 in
      ldglite)
        controlFile="$PWD/obs/debian/control"
        sed '/^Build-Depends:/ s/$/ libosmesa6-dev/' -i $controlFile
        ;;
      ldview)
        controlFile="$PWD/QT/debian/control"
        sed -e '/#Qt4.x/d' -e '/^Build-Depends/d' \
            -e 's/#Build-Depends/Build-Depends/g' -i $controlFile
        if [ "$LP3D_BUILD_APPIMAGE" = "true" ]; then
          sed -e 's/ libkf5kio-dev//g' \
              -e 's/ extra-cmake-modules//g' \
              -e 's/ libkf5kdelibs4support5-bin//g' -i $controlFile
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
      $useSudo apt-get update -qq > $depsLog 2>&1
      $useSudo apt-get install -y $controlDeps >> $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
      *)
      Info "ERROR - Unknown platform [$platform_id]"
      ;;
    esac;
  else
    Info "ERROR - Platform is undefined or invalid [$OS_NAME] - Cannot continue."
  fi
}

# args: <none>
ApplyLDViewStdlibHack(){
  Info "Apply stdlib error patch to LDViewGlobal.pri on $platform_pretty v$([ -n "$platform_ver" ] && [ "$platform_ver" != "undefined" ] && echo $platform_ver || true) ..."
  sed s/'    # detect system libraries paths'/'    # Suppress fatal error: stdlib.h: No such file or directory\n    QMAKE_CFLAGS_ISYSTEM = -I\n\n    # detect system libraries paths'/ -i LDViewGlobal.pri
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDGLite() {
  BUILD_CONFIG="CONFIG+=BUILD_CHECK CONFIG-=debug_and_release"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [[ -n "$build_osmesa" && ! "$get_local_libs" = 1 ]]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  if [ "$no_gallium" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=NO_GALLIUM"
  fi
  if [ "$get_local_libs" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_LOCAL=$LP3D_LL_USR"
  fi
  ${QMAKE_EXEC} -v && Info
  ${QMAKE_EXEC} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make -j${BUILD_CPUs}
    make install
  else
    make -j${BUILD_CPUs} > $2 2>&1
    make install >> $2 2>&1
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDView() {
  # Patch fatal error: stdlib.h: No such file or directory
  # on Docker, Fedora's platform_id is 'fedora', on OBS it is 'redhat'
  case ${platform_id} in
  redhat|fedora|suse)
     case ${platform_ver} in
     24|25|26|27|28|29|30|1500|1550|150000)
       ApplyLDViewStdlibHack
       ;;
     esac
    ;;
  arch)
    ApplyLDViewStdlibHack
    ;;
  ubuntu)
    if [[ "${DOCKER}" = "true" && "${platform_ver}" = "18.04" ]]; then
      ApplyLDViewStdlibHack
    fi
    ;;
  esac
  BUILD_CONFIG="CONFIG+=BUILD_CUI_ONLY CONFIG+=USE_SYSTEM_LIBS CONFIG+=BUILD_CHECK CONFIG-=debug_and_release"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [[ -n "$build_osmesa" && ! "$get_local_libs" = 1 ]]; then
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
  if [ "$build_sdl2" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG --with-libsdl2=from-src"
  else
    BUILD_CONFIG="$BUILD_CONFIG --with-libsdl2"
  fi
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG --enable-debug"
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
  Info "BUILD_FLAGS: $BUILD_FLAGS" && \
  Info "BUILD_CONFIG: $BUILD_CONFIG" && \
  env $BUILD_FLAGS ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." $BUILD_CONFIG || \
  Info "BUILD_CONFIG: $BUILD_CONFIG" && \
  ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." $BUILD_CONFIG
  #Info "DEBUG_DEBUG CONFIG.LOG: " && cat config.log
  if [ "${OBS}" = "true" ]; then
    make -j${BUILD_CPUs}
    make install
    make check
    if [ ! -f "unix/lpub3d_trace_cui" ]; then
      Info "ERROR - lpub3d_trace_cui build failed!"
      Info "The config.log may give some insights."
      cat config.log
    fi
  else
    make -j${BUILD_CPUs} > $2 2>&1 &
    TreatLongProcess "$!" "60" "POV-Ray make"
    make check >> $2 2>&1
    make install >> $2 2>&1
  fi
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

# Grab the calling dir
CallDir=$PWD

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

Info && Info "Building.................[LPub3D 3rd Party Renderers]"

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
  parent_dir=${PWD##*/}
  if  [ "$parent_dir" = "utilities" ]; then
    if [ "$OS_NAME" = "Darwin" ]; then
      chkdir="$PWD/../../../"
    else
      chkdir="$(realpath ../../../)"
    fi
    if [ -d "$chkdir" ]; then
      WD=$chkdir
    fi
  else
    WD=$PWD
  fi
  Info "WARNING - 'WD' environment varialbe not specified. Using $WD"
fi

# Initialize OBS if not in command line input
if [[ "${OBS}" = "" && "${DOCKER}" = "" &&  "${TRAVIS}" = "" ]]; then
  OBS=true
fi

# Get pretty platform name, short platform name and platform version
OS_NAME=$(uname)
if [ "$OS_NAME" = "Darwin" ]; then
  platform_pretty=$(echo `sw_vers -productName` `sw_vers -productVersion`)
  platform_id=macos
  platform_ver=$(echo `sw_vers -productVersion`)
else
  platform_id=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $OS_NAME | awk '{print tolower($0)}')
  platform_pretty=$(. /etc/os-release 2>/dev/null; [ -n "$PRETTY_NAME" ] && echo "$PRETTY_NAME" || echo $OS_NAME)
  platform_ver=$(. /etc/os-release 2>/dev/null; [ -n "$VERSION_ID" ] && echo $VERSION_ID || echo 'undefined')
  if [ "${OBS}" = "true" ]; then
    if [ "$RPM_BUILD" = "true" ]; then
      Info "OBS Build Family.........[RPM_BUILD]"
      if [ -n "$TARGET_VENDOR" ]; then
        platform_id=$TARGET_VENDOR
      else
        Info "WARNING - Open Build Service did not provide a target platform."
        platform_id=$(echo $OS_NAME | awk '{print tolower($0)}')
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
    fi
  fi
  # change Arch Pretty Name export Arch Extra codes
  if [ "$platform_id" = "arch" ]; then
    platform_pretty=$PLATFORM_PRETTY
    platform_ver=$PLATFORM_VER
  fi
fi
[ -n "$platform_id" ] && host=$platform_id || host=undefined

TARGET_CPU=$(uname -m)

# Display platform settings
Info "Working Directory........[${CallDir}]"
Info "Platform ID..............[${platform_id}]"
if [ "$LP3D_BUILD_APPIMAGE" = "true" ]; then
  platform_pretty="AppImage (using $platform_pretty)"
fi
if [ "${DOCKER}" = "true" ]; then
  Info "Platform Pretty Name.....[Docker Container - ${platform_pretty}]"
  if [ "${platform_id}" = "arch" ]; then
     [ -n "$build_osmesa" ] && Info "OSMesa...................[Build from source]"
  fi
elif [ "${TRAVIS}" = "true" ]; then
  Info "Platform Pretty Name.....[Travis CI - ${platform_pretty}]"
elif [ "${OBS}" = "true" ]; then
  if [ "${TARGET_CPU}" = "aarch64" ]; then
    platform_pretty="$platform_pretty (ARM)"
  fi
  Info "Target CPU...............[${TARGET_CPU}]"
  Info "Platform Pretty Name.....[Open Build Service - ${platform_pretty}]"
  [ "$platform_id" = "arch" ] && build_tinyxml=1 || true
  [ -n "$get_qt5" ] && Info "Get Qt5 library..........[$LP3D_QT5_BIN]" || true
  [[ -n "$build_osmesa" && ! -n "$get_local_libs" ]] && Info "OSMesa...................[Build from source]"
  [ -n "$no_gallium" ] && Info "Gallium driver...........[Not available]" || true
  [ -n "$get_local_libs" ] && Info "Get local libraries......[Using OSMesa, LLVM, OpenEXR, and DRM from $LP3D_LL_USR/lib64]" || true
  [ -n "$build_sdl2" ] && Info "SDL2.....................[Build from source]" || true
  [ -n "$build_tinyxml" ] && Info "TinyXML..................[Build from source]" || true
  [ -n "$build_gl2ps" ] && Info "GL2PS....................[Build from source]" || true
else
  Info "Platform Pretty Name.....[${platform_pretty}]"
fi
Info "Platform Version.........[$platform_ver]"

Info "Working Directory........[$WD]"

# Distribution directory
if [ "$OS_NAME" = "Darwin" ]; then
  DIST_DIR=lpub3d_macos_3rdparty
else
  DIST_DIR=lpub3d_linux_3rdparty
fi
DIST_PKG_DIR=${WD}/${DIST_DIR}
if [ ! -d "${DIST_PKG_DIR}" ]; then
  mkdir -p ${DIST_PKG_DIR}
fi
Info "Dist Directory...........[${DIST_PKG_DIR}]"

# Change to Working directory
# Travis: /home/travis/build/trevorsandy
cd ${WD}

# set log output path
LOG_PATH=${WD}
Info "Log Path.................[${LOG_PATH}]"

# Setup LDraw Library - for testing LDView and LDGLite and also used by LPub3D test
if [ "$OS_NAME" = "Darwin" ]; then
  LDRAWDIR_ROOT=${HOME}/Library
else
  LDRAWDIR_ROOT=${HOME}
fi
export LDRAWDIR=${LDRAWDIR_ROOT}/ldraw
if [ "$OBS" != "true" ]; then
  if [ ! -f "${DIST_PKG_DIR}/complete.zip" ]; then
    Info && Info "LDraw archive complete.zip not found at ${DIST_PKG_DIR}. Downloading archive..."
    curl $curlopts http://www.ldraw.org/library/updates/complete.zip -o ${DIST_PKG_DIR}/complete.zip;
  fi
  if [ ! -f "${DIST_PKG_DIR}/lpub3dldrawunf.zip" ]; then
    Info "LDraw archive lpub3dldrawunf.zip not found at ${DIST_PKG_DIR}. Downloading archive..."
    curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o ${DIST_PKG_DIR}/lpub3dldrawunf.zip;
  fi
fi
if [ ! -d "${LDRAWDIR}/parts" ]; then
  if [ "$OBS" != "true" ]; then
    cp -f ${DIST_PKG_DIR}/complete.zip .
  fi
  Info "Extracting LDraw library into ${LDRAWDIR}..."
  unzip -od ${LDRAWDIR_ROOT} -q complete.zip;
  if [ -d "${LDRAWDIR}/parts" ]; then
    Info "LDraw library extracted. LDRAWDIR defined."
  fi
elif [ ! "$OS_NAME" = "Darwin" ]; then
  Info "LDraw Library............[${LDRAWDIR}]"
fi
# Additional LDraw configuration for MacOS
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
  BUILD_CPUs=$(nproc)
fi

# get Qt version
Info && ${QMAKE_EXEC} -v && Info
QMAKE_EXEC="${QMAKE_EXEC} -makefile"

# backup ld_library_path
LP3D_LD_LIBRARY_PATH_SAVED=$LD_LIBRARY_PATH

# initialize mesa build flag
OSMesaBuilt=0

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

# define build architecture and cached renderer paths
VER_LDGLITE=ldglite-1.3
VER_LDVIEW=ldview-4.3
VER_POVRAY=lpub3d_trace_cui-3.8
# distArch=$(uname -m)
#if [[ "$distArch" = "x86_64" || "$distArch" = "aarch64" ]]; then
if [[ "$TARGET_CPU" = "x86_64" || "$TARGET_CPU" = "aarch64" ]]; then
  buildArch="64bit_release";
else
  buildArch="32bit_release";
fi
LP3D_LDGLITE=${DIST_PKG_DIR}/${VER_LDGLITE}/bin/${distArch}/ldglite
LP3D_LDVIEW=${DIST_PKG_DIR}/${VER_LDVIEW}/bin/${distArch}/ldview
LP3D_POVRAY=${DIST_PKG_DIR}/${VER_POVRAY}/bin/${distArch}/lpub3d_trace_cui

#echo && echo "================================================"
#echo "DEBUG - DISTRIBUTION FILES:" && find $DIST_PKG_DIR -type f;
#echo "================================================" && echo

# install build dependencies for MacOS
if [ "$OS_NAME" = "Darwin" ]; then
  Info &&  Info "Install $OS_NAME renderer build dependencies..."
  Info "----------------------------------------------------"
  Info "Platform.................[macos]"
  Info "Using sudo...............[No]"
  for buildDir in ldview povray; do
    artefactBinary="LP3D_$(echo ${buildDir} | awk '{print toupper($0)}')"
    if [ ! -f "${!artefactBinary}" ]; then
      case ${buildDir} in
      ldview)
        brewDeps="tinyxml gl2ps libjpeg minizip"
        ;;
      povray)
        brewDeps="$brewDeps openexr sdl2 libtiff boost autoconf automake pkg-config"
        ;;
      esac
    fi
  done
  depsLog=${LOG_PATH}/${ME}_${host}_deps_$OS_NAME.log
  if [ -n "$brewDeps" ]; then
    Info "Dependencies List...[X11 ${brewDeps}]"
    Info "Checking for X11 (xquartz) at /usr/X11..."
    if [[ -d /usr/X11/lib && /usr/X11/include ]]; then
      Info "Good to go - X11 found."
    else
      Info "ERROR - Sorry to say friend, I cannot go on - X11 not found."
      if [ "${TRAVIS}" != "true" ]; then
        Info "  You can install xquartz using homebrew:"
        Info "  \$ brew cask list"
        Info "  \$ brew cask install xquartz"
        Info "  Note: elevated access password will be required."
      fi
      # Elapsed execution time
      FinishElapsedTime
      exit 1
    fi
    brew update > $depsLog 2>&1
    brew install $brewDeps >> $depsLog 2>&1
    Info "$OS_NAME dependencies installed." && DisplayLogTail $depsLog 10
  else
    Info "Renderer artefacts exist, nothing to build. Install dependencies skipped" > $depsLog 2>&1
    DisplayLogTail $depsLog 3
  fi
  # Set povray --without-optimiz flag on macOS High Sierra 10.13
  [ "$(echo $platform_ver | cut -d. -f2)" = 13 ] && MACOS_POVRAY_NO_OPTIMIZ="true" || true
  # set dependency profiler and nubmer of CPUs
  LDD_EXEC="otool -L"
  BUILD_CPUs=$(sysctl -n hw.ncpu)
fi

# Main loop
for buildDir in ldglite ldview povray; do
  buildDirUpper="$(echo ${buildDir} | awk '{print toupper($0)}')"
  artefactVer="VER_${buildDirUpper}"
  artefactBinary="LP3D_${buildDirUpper}"
  buildLog=${LOG_PATH}/${ME}_${host}_build_${buildDir}.log
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
    curlCommand="https://github.com/trevorsandy/ldview/archive/qmake-build.tar.gz"
    checkString="LDView Image Output"
    linesAfter="9"
    buildCommand="BuildLDView"
    validSubDir="OSMesa"
    validExe="${validSubDir}/${buildArch}/ldview"
    buildType="release"
    displayLogLines=27
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
      Info && Info "ERROR - Unable to find ${buildDir}.tar.gz at $PWD"
    fi
    if [[ -n "$build_osmesa" && ! "$OSMesaBuilt" = 1 && ! "$get_local_libs" = 1 ]]; then
      BuildMesaLibs
    fi
    if [[ "$platform_id" = "suse" && "${buildDir}" = "povray" && $(echo "$platform_ver" | grep -E '1315') ]]; then
      OBS_RPM_BUILD_CFLAGS="$RPM_OPTFLAGS -fno-strict-aliasing -Wno-multichar"
      OBS_RPM_BUILD_CXXFLAGS="$OBS_RPM_BUILD_CFLAGS -std=c++11 -Wno-reorder -Wno-sign-compare -Wno-unused-variable \
      -Wno-unused-function -Wno-comment -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-switch"
      OBS_RPM_BUILD_CONFIG="--disable-dependency-tracking --disable-strip --disable-optimiz --with-boost-libdir=${RPM_LIBDIR}"
      Info && Info "Using RPM_BUILD_FLAGS: $OBS_RPM_BUILD_CXXFLAGS and  OBS_RPM_BUILD_CONFIG: ${OBS_RPM_BUILD_CONFIG}" && Info
      OBS_RPM1315_BUILD_OPTS=1
    fi
  else
    # CI/Local build setup - we must install dependencies even if binary exists...
    if [[ ! -f "${!artefactBinary}" || ! "$OS_NAME" = "Darwin" ]]; then
      # Check if build folder exist - donwload tarball and extract if not
      Info && Info "Setup ${!artefactVer} source files..."
      Info "----------------------------------------------------"
      if [ ! -d "${buildDir}/${validSubDir}" ]; then
        Info && Info "$(echo ${buildDir} | awk '{print toupper($0)}') build folder does not exist. Checking for tarball archive..."
        if [ ! -f ${buildDir}.tar.gz ]; then
          Info "$(echo ${buildDir} | awk '{print toupper($0)}') tarball ${buildDir}.tar.gz does not exist. Downloading..."
          curl $curlopts ${curlCommand} -o ${buildDir}.tar.gz
        fi
        ExtractArchive ${buildDir} ${validSubDir}
      else
        cd ${buildDir}
      fi
      # Install build dependencies
      if [[ ! "$OS_NAME" = "Darwin" && ! "$OBS" = "true" ]]; then
        Info && Info "Install ${!artefactVer} build dependencies..."
        Info "----------------------------------------------------"
        InstallDependencies ${buildDir}
        sleep .5
      fi
    fi
  fi

  # Perform build - only if binary does not exist
  Info && Info "Build ${!artefactVer}..."
  Info "----------------------------------------------------"
  if [ ! -f "${!artefactBinary}" ]; then
    ${buildCommand} ${buildType} ${buildLog}
    [ -f "${validExe}" ] && Info && Info "$LDD_EXEC check ${buildDir}..." && \
    $LDD_EXEC ${validExe} 2>/dev/null || Info "ERROR - $LDD_EXEC ${validExe} failed."
    if [ ! "${OBS}" = "true" ]; then
      if [ -f "${validExe}" ]; then
        Info && Info "Build check - ${buildDir}..."
        DisplayCheckStatus "${buildLog}" "${checkString}" "${linesBefore}" "${linesAfter}"
        Info
        DisplayLogTail ${buildLog} ${displayLogLines}
      else
        Info && Info "ERROR - ${validExe} not found. Binary was not successfully built"
        Info "------------------Build Log-------------------------"
        cat ${buildLog}
      fi
    fi
    Info && Info "Build ${buildDir} finished." && Info
  else
    Info "Renderer artefact binary for ${!artefactVer} exists - build skipped."
  fi
  cd ${WD}
done
# Restore ld_library_path
export LD_LIBRARY_PATH=$LP3D_LD_LIBRARY_PATH_SAVED

# Elapsed execution time
FinishElapsedTime
