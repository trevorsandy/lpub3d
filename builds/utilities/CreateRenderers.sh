#!/bin/bash
#
# Build all LPub3D 3rd-party renderers
#
# Trevor SANDY <trevor.sandy@gmail.com>
# Last Update: September 17, 2025
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

# =======================================
# Functions
# =======================================

# Capture elapsed time - reset BASH time counter
function FinishElapsedTime()
{
  # Elapsed execution time
  local ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# Args: 1 = <start> (seconds mark)
function ElapsedTime()
{
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

# Args: [$1 = -n,] $2 = <message>
function Info()
{
  local i
  if [ "${NEW_LINE}" = 0 ]; then
    NEW_LINE=1
    echo "${*}" >&2
    return
  elif [ "$1" = "-n" ]; then
    NEW_LINE=0
    i=${*:2}
    if [ "${SOURCED}" = "true" ]; then
      f="${0##*/}"; f="${f%.*}"
      echo -n "   ${f}: ${i}" >&2
    else
      echo -n "-${i}" >&2
    fi
    return
  else
    i=${*}
  fi
  if [ "${SOURCED}" = "true" ]; then
    f="${0##*/}"; f="${f%.*}"
    echo "   ${f}: ${i}" >&2
  else
    echo "-${i}" >&2
  fi
}

# Args: $1 = <build folder>, $2 = <valid subfolder>
function ExtractArchive()
{
  Info "Extracting $1.tar.gz..."
  mkdir -p $1 && tar -mxzf $1.tar.gz -C $1 --strip-components=1
  if [ -d $1/$2 ]; then
    Info "Archive $1.tar.gz successfully extracted."
    [ "${LP3D_NO_CLEANUP}" != "true" ] && rm -rf $1.tar.gz && Info "Cleanup archive $1.tar.gz." && Info || :
    cd "$1" || :
  else
    Info "ERROR - $1.tar.gz did not extract properly." && Info
  fi
}

function Version_GT() {
    test "$(printf '%s\n' "$@" | $SORT -V | head -n 1)" != "$1";
}

# Args: $1 = <log file>, $2 = <position>
function DisplayLogTail()
{
  [ "$LP3D_NO_LOG_TAIL" = "true" ] && return 0 || :
  if [[ -f "$1" && -s "$1" ]]; then
    logFile="$1"
    if [ "$2" = "" ]; then
      # Default to 5 lines from the bottom of the file if not specified
      startPosition=-5
    elif [[ "${2:0:1}" != "-" || "${2:0:1}" != "+" ]]; then
      # Default to the bottom of the file if not specified
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

# Args: $1 = <log file>, $2 = <search String>, $3 = <lines Before>, $4 = <lines After>
function DisplayCheckStatus()
{
  declare -i i; i=0
  for arg in "$@"; do
    i=$((i+1))
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

# Args: 1 = <pid>, 2 = <message interval>, [3 = <pretty label>]
function TreatLongProcess()
{
  declare -i i; i=0
  for arg in "$@"; do
    i=$((i+1))
    if test $i -eq 1; then s_pid="$arg"; fi    # pid
    if test $i -eq 2; then s_msgint="$arg"; fi # message interval
    if test $i -eq 3; then s_plabel="$arg"; fi # pretty label
  done

  # Initialize the duration counter
  s_start=$SECONDS

  # Validate the optional pretty label
  if test -z "$s_plabel"; then s_plabel="Create renderer"; fi

  # Spawn a process that coninually reports the command is running
  Info
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
  Info
  return $s_return_code
}

# Args: 1 = <build folder>
function InstallDependencies()
{
  if [ "$OS_NAME" = "Linux" ]; then
    Msg="Install $1 build dependencies for target platform: [$platform_id]..."
    if [ "$LP3D_BUILD_OS" = "appimage" ]; then
      depsLog=${LP3D_LOG_PATH}/${ME}_AppImage_deps_${1}.log
    else
      depsLog=${LP3D_LOG_PATH}/${ME}_${host}_deps_${1}.log
    fi
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
    Info "Platform_id.........[${platform_id}]"
    case ${platform_id} in
    fedora|centos)
      # Initialize install mesa
      case $1 in
      ldglite)
        specFile="$PWD/obs/ldglite.spec"
        ;;
      ldview)
        if which -q qmake6 >/dev/null 2>&1 ; then
          specFile="/tmp/LDView-qt6.spec"
          cp -f $PWD/QT/LDView.spec $specFile
          sed -e 's/define qt6 0/define qt6 1/g' -e 's/kdebase-devel/make/g' -e 's/, kdelibs-devel//g' -i $specFile
        else
          specFile="/tmp/LDView-qt5.spec"
          cp -f $PWD/QT/LDView.spec $specFile
          sed -e 's/define qt5 0/define qt5 1/g' -e 's/kdebase-devel/make/g' -e 's/, kdelibs-devel//g' -i $specFile
        fi
        ;;
      povray)
        specFile="$PWD/unix/obs/povray.spec"
       ;;
      esac;
      rpmbuildDeps=$(rpmbuild --nobuild ${specFile} 2>&1 | grep 'needed by'| awk ' {print $1}')
      Info "Spec File...........[${specFile}]"
      Info "Dependencies List...[${rpmbuildDeps}]"
      if [[ -n "$build_osmesa" && "$MesaBuildAttempt" != 1 ]]; then
        BuildMesaLibs $useSudo
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
        pkgbuildFile="$PWD/QT/PKGBUILD"
        if which -q qmake6 >/dev/null 2>&1 ; then
          sed -e 's/qt5/qt6/g' -e 's/qmake /qmake6 /g' -e '/^depends/s/)/ '\''qt6-5compat'\'')/' -i $pkgbuildFile
        fi
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
      pkgbuildDeps="inetutils ${pkgbuildDeps}"
      Info "PKGBUILD File.......[${pkgbuildFile}]"
      Info "Dependencies List...[${pkgbuildDeps}]"
      Info
      $useSudo pacman -Syy --noconfirm --needed >> $depsLog 2>&1
      $useSudo pacman -Syu --noconfirm --needed >> $depsLog 2>&1
      $useSudo pacman -S --noconfirm --needed $pkgbuildDeps >> $depsLog 2>&1
      if [[ -n "$build_osmesa" && "$MesaBuildAttempt" != 1 ]]; then
        BuildMesaLibs $useSudo
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
        sed -e '/^#Qt/d' -e '/libqt4-dev/d' -i $controlFile
        if which -q qmake6 >/dev/null 2>&1 ; then
          sed -e '/qtbase5-dev/d' -e 's/#Build-Depends/Build-Depends/g' -i $controlFile
        else
          sed -e '/qt6-base-dev/d' -e 's/#Build-Depends/Build-Depends/g' -i $controlFile
        fi
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

# Args: <none>
function ApplyLDViewStdlibHack()
{
  Info -n "Apply stdlib error patch to LDViewGlobal.pri on $platform_pretty v$([ -n "$platform_ver" ] && [ "$platform_ver" != "undefined" ] && echo $platform_ver || :)..."
  (sed s/'    # detect system libraries paths'/'    # Suppress fatal error: stdlib.h: No such file or directory\n    QMAKE_CFLAGS_ISYSTEM = -I\n\n    # detect system libraries paths'/ -i LDViewGlobal.pri
  ) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
  Info
}

# Args: 1 = <sudo>
function BuildMesaLibs()
{
  return_code=0
  lib_ext=a
  lib_file=OSMesa32
  lib_dir="$(test -d ${RPM_LIBDIR} && echo $(basename ${RPM_LIBDIR}) || echo lib)"
  build_label="${lib_file} and GLU"
  mesaUtilsDir="$CALL_DIR/builds/utilities/mesa"

  Info && Info "Build ${lib_file} libraries..." && Info
  if [[ "${OBS}" != "true" && "$LP3D_NO_DEPS" != "true" ]]; then
    Info "Using sudo............[$([ -n "$2" ] && echo "Yes" || echo "No")]"
    mesaDepsLog=${LP3D_LOG_PATH}/${ME}_${host}_mesadeps_${1}.log

    if [ -z "$1" ]; then
      useSudo=
    else
      useSudo=$1
    fi

    case ${platform_id} in
    fedora|redhat|centos|suse|mageia|openeuler)
      mesaBuildDeps="See ${mesaDepsLog}..."
      Info "RPM ${lib_file} Dependencies.[${mesaBuildDeps}]"
      Info "GLU Spec File.......[${mesaUtilsDir}/glu.spec]"
      Info
      $useSudo dnf builddep -y mesa >> $mesaDepsLog 2>&1
      Info "RPM Mesa dependencies installed." && DisplayLogTail $mesaDepsLog 10
      $useSudo dnf builddep -y "${mesaUtilsDir}/glu.spec" >> $mesaDepsLog 2>&1
      Info "RPM GLU dependencies installed." && DisplayLogTail $mesaDepsLog 5
      ;;
    arch)
      pkgbuildFile="${mesaUtilsDir}/PKGBUILD"
      mesaBuildDeps="$(echo `grep -wr 'depends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'` \
                            `grep -wr 'makedepends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'`)"
      Info "PKG Linux ${lib_file} Dependencies.[${mesaBuildDeps}]"
      Info "PKGBUILD File.......[$pkgbuildFile]"
      Info
      $useSudo pacman -S --noconfirm --needed $mesaBuildDeps >> $mesaDepsLog 2>&1
      Info "PKG Linux ${lib_file} and GLU dependencies installed."  && DisplayLogTail $mesaDepsLog 15
      ;;
    debian|ubuntu)
      controlFile="${mesaUtilsDir}/control"
      mesaBuildDeps="$(grep Build-Depends $controlFile | cut -d: -f2| sed 's/(.*)//g' | tr -d ,)"
      Info "DEB Linux ${lib_file} Dependencies.[${mesaBuildDeps}]"
      Info "Control File.......[$controlFile]"
      Info
      $useSudo apt-get update -qq >> $mesaDepsLog 2>&1
      $useSudo apt-get build-dep -y mesa >> $mesaDepsLog 2>&1
      #$useSudo apt-get install -y $mesaBuildDeps >> $mesaDepsLog 2>&1
      Info "DEB Linux ${lib_file} and GLU dependencies installed."  && DisplayLogTail $mesaDepsLog 15
      ;;
    esac
  fi

  # build OSMesa from Mesa-amber
  if [ "${mesa_amber}" = 1 -o "${platform_id}" = "arch" ]; then
    mesa_version=21.3.9
    glu_version=9.0.1
    zstd_version=1.5.7
    llvm_not_used=1
    build_label="${lib_file}, GLU and ZStd"
  else
    mesa_version=17.2.6
    glu_version=9.0.0
    unset zstd_version
    llvm_not_used=0
  fi
  if [ "${OBS}" = "true" ]; then
    Info "Using sudo...............[No]"
    if [[ ("${platform_id}" = "redhat" && ${platform_ver} = 28) || \
          ("${platform_id}" = "debian" && ${platform_ver} = 10) || \
           "${LP3D_UCS_VER}" != "" ]]; then
      mesa_version=18.3.5
    fi
  else
    mesaBuildLog=${LP3D_LOG_PATH}/${ME}_${host}_mesabuild.log
  fi

  Info "Building ${lib_file}........[Mesa$([ "${mesa_amber}" = 1 ] && echo "-amber ${mesa_version}" || echo " ${mesa_version}")]"
  mesa_prefix=${DIST_PKG_DIR}/mesa/${platform_id}

  # This block will support future use case to build LLVM
  if Version_GT 21.3.0 "$mesa_version"; then
    if [ -f "${llvm_config}" ]; then
      Info "Installed LLVM version...[${llvm_version}]"
      llvm_max_ver=14.99
      if Version_GT "${llvm_version}" "${llvm_max_ver}"; then
        Info "Installed LLVM ${llvm_version} exceeds the maximum supported version ${llvm_max_ver} for Mesa ${mesa_version}."
        if Version_GT 18.0.0 "$mesa_version"; then
          Info "LLVM 4.0.1 is the best option for Mesa 17.x."
        elif Version_GT 19.0.0 "$mesa_version"; then
          Info "LLVM 6.0.1 is the best option for Mesa 18.x."
          Info "LLVM 15 and greater will NOT work, because of:"
          Info " https://discourse.llvm.org/t/rfc-remove-most-constant-expressions/63179/11 (LLVM 15)"
          Info " https://llvm.org/docs/OpaquePointers.html#frontends (LLVM 16)"
        fi
        exit 1 # Exit until build LLVM module is added to build_mesa.sh
        Info "Building LLVM version....[${llvm_max_ver}]"
        unset llvm_config
        llvm_build=1
        llvm_prefix=${DIST_PKG_DIR}/llvm
        if [ ! -d "${llvm_prefix}" ]; then
          mkdir -p "${llvm_prefix}"
        fi
      fi
    fi
  fi

  MESA_OPTIONS="\
  OBS=${OBS} \
  LIB_DIR=${lib_dir} \
  RPM_BUILD=${RPM_BUILD} \
  LLVM_NOT_USED=${llvm_not_used} \
  LLVM_CONFIG=${llvm_config} \
  LLVM_PREFIX=${llvm_prefix} \
  LLVM_BUILD=${llvm_build} \
  MESA_PREFIX=${mesa_prefix} \
  MESA_VERSION=${mesa_version} \
  GLU_VERSION=${glu_version} \
  ZSTD_VERSION=${zstd_version}\
  "

  chmod a+x "${mesaUtilsDir}/build_mesa.sh"
  if [ "${OBS}" = "true" ]; then
    Info
    env ${MESA_OPTIONS} ${mesaUtilsDir}/build_mesa.sh
  else
    Info "${lib_file} Build Log.......[${mesaBuildLog}]" && Info
    env ${MESA_OPTIONS} ${mesaUtilsDir}/build_mesa.sh > ${mesaBuildLog} 2>&1 &
    TreatLongProcess "$!" "60" "${build_label} build"
  fi

  local return_code=$?
  if [ "$return_code" -ne 0 ]; then
    MesaBuildAttempt=1
    Info && Info "ERROR - build_mesa.sh failed with return code ${return_code}. Binary was not built."
    return $return_code
  else
    export PKG_CONFIG_PATH="${mesa_prefix}/${lib_dir}/pkgconfig:$PKG_CONFIG_PATH"
    export LD_LIBRARY_PATH="${mesa_prefix}/${lib_dir}:$LD_LIBRARY_PATH"
    Info "Set LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
  fi

  if [ ! -f "${mesa_prefix}/${lib_dir}/lib${lib_file}.${lib_ext}" ]; then
    Info && Info "ERROR - lib${lib_file}.${lib_ext} not found. Binary was not successfully built."
    return_code=1
  fi
  if [ ! -f "${mesa_prefix}/${lib_dir}/libGLU.a" ]; then
    Info "ERROR - libGLU.a not found. Binary was not successfully built."
    return_code=2
  fi
  if [[ -n "${zstd_version}" && ! -f "${mesa_prefix}/${lib_dir}/libzstd.a" ]]; then
    Info "ERROR - libzstd.a not found. Binary was not successfully built."
    return_code=3
  fi

  if [ "$return_code" -eq 0 ]; then
    if [ "${OBS}" != "true" ]; then
      Info && Info "${build_label} libs build check..."
      DisplayCheckStatus "$mesaBuildLog" "Libraries have been installed in:" "1" "16"
      DisplayLogTail $mesaBuildLog 20
    fi
    MesaBuildAttempt=1
  else
    if [ "${OBS}" != "true" ]; then
      Info "------------------Build Log-------------------------"
      cat $mesaBuildLog
    fi
  fi
  Info && Info "${lib_file} libraries build finished."
  return $return_code
}

# Args: 1 = <build type (release|debug)>, 2 = <build log>
function BuildLDGLite()
{
  BUILD_CONFIG="CONFIG+=BUILD_CHECK CONFIG-=debug_and_release"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [[ -n "$build_osmesa" && "$get_local_libs" != 1 ]]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  if [ "$llvm_not_used" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=OSMESA_NO_LLVM"
  fi
  if [ "$get_local_libs" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_LOCAL=$LP3D_LL_USR"
  fi
  if [ "$local_freeglut" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_FREEGLUT_LOCAL"
  fi
  BUILD_CONFIG="CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}"
  [ "${MSYS2}" = 1 ] && BUILD_CONFIG="PREFIX=${MINGW_PREFIX} ${BUILD_CONFIG} CONFIG+=msys" || :
  ${QMAKE_EXEC} -v && Info
  ${QMAKE_EXEC} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make -j${CPU_CORES}
    make install
  else
    make -j${CPU_CORES} > $2 2>&1
    make install >> $2 2>&1
  fi
}

# Args: 1 = <build type (release|debug)>, 2 = <build log>
function BuildLDView()
{
  # Patch fatal error: stdlib.h: No such file or directory
  # on Docker, Fedora's platform_id is 'fedora', on OBS it is 'redhat'
  case ${platform_id} in
  redhat|centos|fedora|suse|mageia|openeuler)
     case ${platform_ver} in
     7|8|9|33|34|36|37|39|40|41|800|1500|1550|150000|150200|150300|2003|2103)
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
  BUILD_CONFIG="$BUILD_CONFIG CONFIG+=static CONFIG+=BUILD_CUI_ONLY CONFIG+=USE_SYSTEM_LIBS"
  if [ "$prebuilt_3ds" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_3RD_PARTY_PREBUILT_3DS"
  fi
  if [ "$build_tinyxml" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=BUILD_TINYXML"
  fi
  if [ "$build_gl2ps" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=BUILD_GL2PS"
  fi
  BUILD_CONFIG="$BUILD_CONFIG CONFIG+=NO_HEADERS_INSTALL"
  if [[ -n "$build_osmesa" && "$get_local_libs" != 1 ]]; then
    if [ "$llvm_not_used" = 1 ]; then
      BUILD_CONFIG="$BUILD_CONFIG CONFIG+=OSMESA_NO_LLVM"
    fi
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  if [ "$get_local_libs" = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_LOCAL=$LP3D_LL_USR"
  fi
  if [ "${MSYS2}" = 1 ]; then
    if [ "${LP3D_LDVIEW_CUI_OPT}" = "Qt" ]; then
      LP3D_PATH="../.."
      BUILD_CONFIG="${BUILD_CONFIG} CONFIG+=CUI_QT"
    elif [ "${LP3D_LDVIEW_CUI_OPT}" = "WGL" ]; then
      BUILD_CONFIG="${BUILD_CONFIG} CONFIG+=CUI_WGL"
      LP3D_PATH=".."
    fi
    BUILD_CONFIG="CONFIG+=3RD_PARTY_INSTALL=${LP3D_PATH}/${DIST_DIR} ${BUILD_CONFIG}"
    BUILD_CONFIG="PREFIX=${MINGW_PREFIX} ${BUILD_CONFIG} CONFIG+=msys"
  else
    BUILD_CONFIG="CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}"
  fi
  #Info "DEBUG_BUILD_CONFIG: ${BUILD_CONFIG}" && Info
  ${QMAKE_EXEC} -v && Info
  ${QMAKE_EXEC} ${BUILD_CONFIG} LDView.pro
  if [ "${OBS}" = "true" ]; then
    make -j${CPU_CORES}
    make install
  else
    make -j${CPU_CORES} > $2 2>&1 &
    TreatLongProcess "$!" "60" "LDView make"
    make install >> $2 2>&1
  fi
}

# Args: 1 = <build type (release|debug)>, 2 = <build log>
function BuildPOVRay()
{
  BUILD_CONFIG="--prefix=${DIST_PKG_DIR} LPUB3D_3RD_PARTY=yes --enable-watch-cursor"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG --enable-debug"
  fi
  if [ "$MACOS_POVRAY_NO_XWINDOW" = "true" ]; then
    BUILD_CONFIG="$BUILD_CONFIG --without-x"
  elif [ "$build_sdl2" = 1 ]; then
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
    Info "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
  fi
  #Info "DEBUG_BUILD_FLAGS:  $BUILD_FLAGS"
  #Info "DEBUG_BUILD_CONFIG: $BUILD_CONFIG"
  [ ! "$OS_NAME" = "Darwin" ] && export POV_IGNORE_SYSCONF_MSG="yes" || :
  chmod a+x unix/prebuild3rdparty.sh && ./unix/prebuild3rdparty.sh > $2 2>&1
  if [ -n "$BUILD_FLAGS" ]; then
    Info && Info "BUILD_FLAGS: $BUILD_FLAGS" && \
    Info "BUILD_CONFIG: $BUILD_CONFIG" && \
    env $BUILD_FLAGS ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." $BUILD_CONFIG >> $2 2>&1
  else
    Info && Info "BUILD_CONFIG: $BUILD_CONFIG" && \
    ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." $BUILD_CONFIG >> $2 2>&1
  fi
  #Info "DEBUG_CONFIG.LOG: " && cat config.log || :
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
    make -j${CPU_CORES} >> $2 2>&1 &
    TreatLongProcess "$!" "60" "POV-Ray make"
    make check >> $2 2>&1
    make install >> $2 2>&1
  fi
}

# Package the renderers
function PackageRenderers()
{
  Info
  if [[ "${OBS}" = "true" || -n "${SNAP}" || -n "${MSYS2}" ]]; then
    Info "Cannot create renderer package under OBS, SNAP or MSYS builds"
    return
  fi
  if [ -d "/out" ]; then
    LP3D_OUT_PATH=/out
  elif [ -d "/buildpkg" ]; then
    LP3D_OUT_PATH=/buildpkg
  elif [ -d "${LP3D_LOG_PATH}" ]; then
    LP3D_OUT_PATH=${LP3D_LOG_PATH}
  else
    LP3D_OUT_PATH=${WD}
  fi
  RENDERERS=3
  declare -r p=Package
  LP3D_ARCH=${TARGET_CPU}
  LP3D_BASE=${platform_id}-${platform_ver}
  LP3D_RNDR_VERSION=${LP3D_VERSION}.${LP3D_VER_REVISION}.${LP3D_VER_BUILD}
  LP3D_RENDERER_ZIP=LPub3D-${LP3D_RNDR_VERSION}-renderers-${LP3D_BASE}-${LP3D_ARCH}.tar.gz
  LP3D_LDVIEW_DEV=--exclude=${VER_LDVIEW}/resources/m6459.ldr
  if [ -f "${LP3D_LDGLITE}" ]; then
    Info "Packaging ${VER_LDGLITE} renderer assets."
    LP3D_RENDERER_PACKAGES="${VER_LDGLITE}"
  else
    Info "WARNING - ${LP3D_LDGLITE} was not found."
    (( RENDERERS-=1 ))
  fi
  if [ -f "${LP3D_POVRAY}" ]; then
    Info "Packaging ${VER_POVRAY} renderer assets."
    LP3D_RENDERER_PACKAGES+=" ${VER_POVRAY}"
  else
    Info "WARNING - ${LP3D_POVRAY} was not found."
    (( RENDERERS-=1 ))
  fi
  if [ -f "${LP3D_LDVIEW}" ]; then
    Info "Packaging ${VER_LDVIEW} renderer assets."
    LP3D_RENDERER_PACKAGES+=" ${VER_LDVIEW}"
    if [ "${LP3D_PACKAGE_LDVQT_DEV}" = "true" ]; then
      Info "Packaging ${VER_LDVIEW} LDVQt dev assets."
    else
      LP3D_LDVIEW_DEV="\
      --exclude=${VER_LDVIEW}/lib \
      --exclude=${VER_LDVIEW}/include \
      --exclude=${VER_LDVIEW}/resources/*Messages.ini"
      if [ "${MSYS2}" = 1 ]; then
        LP3D_LDVIEW_DEV+="\
          --exclude=${VER_LDVIEW}/bin/${TARGET_CPU_QMAKE}/*.exp \
          --exclude=${VER_LDVIEW}/bin/${TARGET_CPU_QMAKE}/*.lib"
      fi
    fi
  else
    Info "WARNING - ${LP3D_LDVIEW} was not found."
    (( RENDERERS-=1 ))
  fi
  if [ "${RENDERERS}" -eq 0 ]; then
    Info "ERROR - Cannot package renderers. No renderers were found."
  else
    Info -n "Creating renderer package ${LP3D_OUT_PATH}/${LP3D_RENDERER_ZIP}..."
    ( cd ${DIST_PKG_DIR} || return && \
    tar -czf ${LP3D_RENDERER_ZIP} ${LP3D_LDVIEW_DEV} $(echo ${LP3D_RENDERER_PACKAGES} | xargs) && \
    sha512sum ${LP3D_RENDERER_ZIP} > ${LP3D_RENDERER_ZIP}.sha512 && \
    mv -f ${LP3D_RENDERER_ZIP} ${LP3D_RENDERER_ZIP}.sha512 \
    ${LP3D_OUT_PATH} ) >$p.out 2>&1 && rm $p.out
    [ -f $p.out ] && Info "ERROR" && tail -80 $p.out || Info "Ok"
    Info
  fi
}

# =======================================
# Main Script
# =======================================

# Elapsed time in seconds
SECONDS=0

# Enable -n (no NEW_LINE termination) flag on Info
NEW_LINE=1

# Grab the calling dir
CALL_DIR=$PWD

# Tell curl to be silent, continue downloads and follow redirects
CURL_OPTS="-sL -C -"

# Populate the OS Name
OS_NAME=$(uname)

# Get CPU arch - 'uname -m' returns x86_64, armv7l or aarch64
TARGET_CPU=$(uname -m)

# Grab the script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# Start message and set sourced and new line flags
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

SORT=sort
if [ "$OS_NAME" = "Darwin" ]; then
  OS_VER=$(uname -r | awk -F . '{print $1}')
  [ "$OS_VER" -le 10 ] && SORT=gsort || :
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

# QMake CPU value for ARM 64bit is arm64
TARGET_CPU_QMAKE=${TARGET_CPU}
if [ "${TARGET_CPU}" = "aarch64" ]; then
  TARGET_CPU_QMAKE="arm64"
fi

# Initialize OBS if not in command line input - FlatPak does not call this script
if [[ -z "${OBS}" && -z "${DOCKER}" && -z "${CI}" && -z "${SNAP}" && -z "${MSYS2}" ]]; then
  OBS=true
fi

Info && Info "Building.................[LPub3D 3rd Party Renderers]"

# Expose GitHub Actions variables
if [[ -n "$CD" || -n "${GITHUB}" ]]; then
  Info
  [ -n "$CI" ] && Info "CI.......................[${CI}]" || :
  [ -n "$OBS" ] && Info "OBS......................[${OBS}]" || :
  [ -n "$DOCKER" ] && Info "Docker...................[${DOCKER}]" || :
  [ -n "$GITHUB" ] && Info "GitHub...................[${GITHUB}]" || :
  [ -n "$LP3D_BASE" ] && Info "Build base...............[${LP3D_BASE}]" || :
  [ -n "$LP3D_ARCH" ] && Info "Build Architecture.......[${LP3D_ARCH}]" || :
  [ -n "$LP3D_APPIMAGE" ] && Info "AppImage.................[${LP3D_APPIMAGE}]" || :
fi

[ -n "$LPUB3D" ] && Info "LPub3D Build Folder......[$LPUB3D]" || :
[ -n "$importedRendererVars" ] && Info "Renderer Build Variables.[rendererVars.sh]" || :

# Get pretty platform name, short platform name and platform version
if [ "$OS_NAME" = "Darwin" ]; then
  platform_pretty=$(echo `sw_vers -productName` `sw_vers -productVersion`)
  platform_id=macos
  platform_ver=$(echo `sw_vers -productVersion`)
else
  platform_id=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $OS_NAME | awk '{print tolower($0)}') #'
  [ "${platform_id}" = "arch" ] && build_osmesa=1 || :
  [[ "${platform_id}" == "msys2" && -z "${MSYS2}" ]] && MSYS2=1 || :
  if [[ -n "${MSYS2}" && -n "${PLATFORM_PRETTY}" ]]; then
    platform_pretty=${PLATFORM_PRETTY}
  else
    platform_pretty=$(. /etc/os-release 2>/dev/null; [ -n "$PRETTY_NAME" ] && echo "$PRETTY_NAME" || echo $OS_NAME)
  fi
  if [ -n "${MSYS2}" ]; then
    if [ -n "${PLATFORM_VER}" ]; then
      platform_ver=${PLATFORM_VER}
    else
      platform_ver=$(echo `uname -a` | awk '{print $3}')
    fi
  else
    platform_ver=$(. /etc/os-release 2>/dev/null; [ -n "$VERSION_ID" ] && echo $VERSION_ID || echo `uname -a`)
  fi
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
  else
    if [ "$RPM_BUILD" = "true" ]; then
      Info "Build Family.............[RPM_BUILD]"
    fi
  fi
  # Change Arch Pretty Name export Arch Extra codes
  if [ "$platform_id" = "arch" ]; then
    platform_pretty=$PLATFORM_PRETTY
    platform_ver=$PLATFORM_VER
  fi
fi
[ -n "$platform_id" ] && host=$platform_id || host=undefined

# Display platform settings
Info "Build Working Directory..[${CALL_DIR}]"
[ -n "${LP3D_UCS_VER}" ] && \
Info "Platform ID..............[ucs]" || \
Info "Platform ID..............[${platform_id}]"
if [ "$LP3D_BUILD_OS" = "snap" ]; then
  platform_pretty="Snap (using $platform_pretty)"
  SNAP="true"
elif [ "$LP3D_BUILD_OS" = "appimage" ]; then
  platform_pretty="AppImage (using $platform_pretty)"
fi

# Until LDView converts to tinyxml2, build tinyxml from source
[ -z "$build_tinyxml" ] && build_tinyxml=1 || true
if [ "${DOCKER}" = "true" ]; then
  Info "Platform Pretty Name.....[Docker Container - ${platform_pretty}]"
elif [ "${CI}" = "true" ]; then
  Info "Platform Pretty Name.....[CI - ${platform_pretty}]"
elif [ "${OBS}" = "true" ]; then
  if [[ "${TARGET_CPU}" = "aarch64" || "${TARGET_CPU}" = "arm7l" ]]; then
    platform_pretty="$platform_pretty (ARM-${TARGET_CPU})"
  fi
  Info "Platform Pretty Name.....[Open Build Service - ${platform_pretty}]"
  [ -n "$prebuilt_3ds" ] && prebuilt_3ds_msg="Use pre-built library" || prebuilt_3ds_msg="Build from source"
  [ -n "$get_qt5" ] && Info "Get Qt5 Library..........[$LP3D_QT_BIN]" || true
  [ -n "$local_freeglut" ] && Info "Freeglut.................[Using local Freeglut]" || true
  [ -n "$get_local_libs" ] && Info "Get Local Libraries......[Using OSMesa, LLVM, OpenEXR, and DRM from $LP3D_LL_USR/lib64]" || true
  [ -n "$build_sdl2" ] && Info "SDL2.....................[Build from source]" || true
else
  Info "Platform Pretty Name.....[${platform_pretty}]"
fi

# LLVM and Mesa configuration
llvm_config=$(which llvm-config 2> /dev/null)
if [ -f "${llvm_config}" ]; then
  llvm_version="$($llvm_config --version | grep -E '^[0-9.]+')"
  llvm_libs_msg="Use system libraries"
else
  llvm_libs_msg="Not available"
fi
if [[ "$build_osmesa" = 1 && "$get_local_libs" != 1 ]]; then
  build_mesa_msg="Build from source"
  [ "$mesa_amber" = 1 ] && \
  build_mesa_msg="${build_mesa_msg} using Mesa-amber" || :
  # can add build_llvm flag to this condition
  [[ "$llvm_not_used" = 1 || "$mesa_amber" = 1 ]] && \
  llvm_libs_msg="Not used for default OSMesa configuration" || :
elif [ "$get_local_libs" = 1 ]; then
  build_mesa_msg="Use local libraries"
  llvm_libs_msg="Use local libraries"
elif [ "$OS_NAME" = "Darwin" ]; then
  build_mesa_msg="Use system libraries"
else
  [ "$(ldconfig -p | grep -i libOSMesa 2>/dev/null)" ] && \
  build_mesa_msg="Use system libraries" || build_mesa_msg="Not found"
fi
[ -n "$build_mesa_msg" ] && Info "OSMesa...................[${build_mesa_msg}]" || true
[ -n "$llvm_libs_msg" ] && Info "LLVM Libraries...........[${llvm_libs_msg}]" || true
[ -n "$build_tinyxml" ] && Info "TinyXML..................[Build from source]" || true
[ -n "$prebuilt_3ds" ] && prebuilt_3ds_msg="Use pre-built library" || prebuilt_3ds_msg="Build from source"
[ -n "$prebuilt_3ds_msg" ] && Info "3DS......................[${prebuilt_3ds_msg}]" || true
if [ -n "$build_gl2ps" ]; then
  Info "GL2PS....................[Build from source]"
  Info "Png......................[Build from source]"
fi

# Set build type
[ -z "${LP3D_BUILD_CONFIG}" ] && LP3D_BUILD_CONFIG="Release" || true
tempvar="$(echo ${LP3D_BUILD_CONFIG} | awk '{print tolower($0)}')"
[[ "$tempvar" != "release" && "$tempvar" != "debug" ]] && \
Info "Build Configuration......[Invalid value: ${LP3D_BUILD_CONFIG}, using 'Release']" && \
tempvar="release" || \
Info "Build Configuration......[${LP3D_BUILD_CONFIG}]"
LP3D_BUILD_CONFIG="${tempvar}" && unset tempvar

Info "Platform Version.........[$platform_ver]"
Info "No Lib Dependency Load...[${LP3D_NO_DEPS:-false}]"
Info "No Display Log Tail......[${LP3D_NO_LOG_TAIL:-true}]"
Info "No Build Cleanup.........[${LP3D_NO_CLEANUP:-false}]"
Info "Working Directory (WD)...[$WD]"

# Set log output path
[ -z "${LP3D_LOG_PATH}" ] && \
LP3D_LOG_PATH=${LP3D_LOG_PATH:-$WD} || :
Info "Log Path.................[${LP3D_LOG_PATH}]"

# Set package renderers flag
[ -z "${LP3D_PUBLISH_RENDERERS}" ] && \
LP3D_PACKAGE_RENDERERS=${LP3D_PUBLISH_RENDERERS:-false} || \
LP3D_PACKAGE_RENDERERS=${LP3D_PUBLISH_RENDERERS}
Info "Package Renderers........[${LP3D_PACKAGE_RENDERERS}]"

# Include LDView libraries in packaged renderers
if [ "${LP3D_PACKAGE_RENDERERS}" == "true" ]; then
  [ -z "${LP3D_PACKAGE_LDVQT_DEV}" ] && \
  LP3D_PACKAGE_LDVQT_DEV=${LP3D_PACKAGE_LDVQT_DEV:-false} || \
  LP3D_PACKAGE_LDVQT_DEV=${LP3D_PACKAGE_LDVQT_DEV}
  Info "Package LDVQt Dev Assets.[${LP3D_PACKAGE_LDVQT_DEV}]"
fi

# Define build architecture and cached renderer paths
if [[ "$TARGET_CPU" = "x86_64" || "$TARGET_CPU" = "aarch64" || "$TARGET_CPU" = "arm64" ]]; then
  BUILD_ARCH="64bit_${LP3D_BUILD_CONFIG}"
else
  BUILD_ARCH="32bit_${LP3D_BUILD_CONFIG}"
fi

# Distribution directory
DIST_DIR=${LP3D_3RD_DIST_DIR:-}
if [ -z "$DIST_DIR" ]; then
  if [ "$OS_NAME" = "Darwin" ]; then
    DIST_DIR=lpub3d_macos_3rdparty
  elif [ -n "${MSYS2}" ]; then
    DIST_DIR=lpub3d_msys_3rdparty
  else
    DIST_DIR=lpub3d_linux_3rdparty
  fi
fi
if [ -z "${LP3D_DIST_DIR_PATH}" ]; then
  DIST_PKG_DIR=${WD}/${DIST_DIR}
else
  DIST_PKG_DIR=${LP3D_DIST_DIR_PATH}
fi
if [ ! -d "${DIST_PKG_DIR}" ]; then
  mkdir -p ${DIST_PKG_DIR}
fi
export DIST_PKG_DIR="${DIST_PKG_DIR}"
Info "Dist Package Directory...[${DIST_PKG_DIR}]"
EXTRAS_DIR=${PWD}/mainApp/extras
Info "Extras Directory.........[${EXTRAS_DIR}]"

# Change to Working directory
# Travis: /home/travis/build/trevorsandy
cd "${WD}" || :

case ${LP3D_LDVIEW_CUI_OPT} in
ldviewqt)
  LP3D_LDVIEW_CUI_OPT=Qt ;;
ldviewosmesa)
  LP3D_LDVIEW_CUI_OPT=OSMesa ;;
ldviewwgl)
  LP3D_LDVIEW_CUI_OPT=WGL ;;
*)
  LP3D_LDVIEW_CUI_OPT=Default ;;
esac
Info "LDView CUI Graphics......[${LP3D_LDVIEW_CUI_OPT}]"

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

declare -r l=Log
LP3D_GITHUB_URL="https://github.com/trevorsandy"
if [ "$OBS" != "true" ]; then
  LP3D_LIBS_BASE=${LP3D_GITHUB_URL}/lpub3d_libs/releases/download/v1.0.1
  if [ ! -f "${DIST_PKG_DIR}/complete.zip" ]; then
    Info && Info "LDraw archive complete.zip not found at ${DIST_PKG_DIR}."
    if [ ! -f "complete.zip" ]; then
      Info -n "Downloading complete.zip..."
      (curl $CURL_OPTS ${LP3D_LIBS_BASE}/complete.zip -o ${DIST_PKG_DIR}/complete.zip) >$l.out 2>&1 && rm $l.out
      [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
    else
      ldrawlib=$(echo $PWD/complete.zip)
      Info -n "Linking complete.zip..."
      (cd "${DIST_PKG_DIR}" && ln -sf "${ldrawlib}" complete.zip) >$l.out 2>&1 && rm $l.out
      [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
    fi
  fi
  if [ ! -f "${DIST_PKG_DIR}/lpub3dldrawunf.zip" ]; then
    Info "LDraw archive lpub3dldrawunf.zip not found at ${DIST_PKG_DIR}."
    if [ ! -f "${EXTRAS_DIR}/lpub3dldrawunf.zip" ]; then
      Info -n "Downloading lpub3dldrawunf.zip..."
      (curl $CURL_OPTS ${LP3D_LIBS_BASE}/lpub3dldrawunf.zip -o ${DIST_PKG_DIR}/lpub3dldrawunf.zip) >$l.out 2>&1 && rm $l.out
      [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
    else
      ldrawlib=$(echo ${EXTRAS_DIR}/lpub3dldrawunf.zip)
      Info -n "Linking lpub3dldrawunf.zip..."
      (cd "${DIST_PKG_DIR}" && ln -sf "${ldrawlib}" lpub3dldrawunf.zip || :) >$l.out 2>&1 && rm $l.out
      [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
    fi
  fi
fi

# LDraw path
if [ ! -d "${LDRAWDIR}/parts" ]; then
  if [ "$OBS" != "true" ]; then
    [ ! -f "complete.zip" ] && \
    cp -f ${DIST_PKG_DIR}/complete.zip . || :
  fi
  Info && Info -n "Extracting ${PWD}/complete.zip LDraw library into ${LDRAWDIR}..."
  ( unzip -od ${LDRAWDIR_ROOT} -q complete.zip; ) >$l.out 2>&1 && rm $l.out
  [ -f $l.out ] && Info "ERROR - Failed." && tail -20 $l.out || Info "Ok"
  if [ -d "${LDRAWDIR}/parts" ]; then
    Info "LDraw library extracted. LDRAWDIR defined." && Info
  fi
elif [ "$OS_NAME" != "Darwin" ]; then
  Info "LDraw Library............[${LDRAWDIR}]"
fi

# QMake, LDD and CPU cores configuration

if [ "$OS_NAME" = "Darwin" ]; then
  # Additional macOS LDraw configuration
  Info "LDraw Library............[${LDRAWDIR}]"
  Info && Info "set LDRAWDIR in environment.plist..."
  chmod +x ${LPUB3D}/builds/utilities/set-ldrawdir.command && ./${LPUB3D}/builds/utilities/set-ldrawdir.command
  grep -A1 -e 'LDRAWDIR' ~/.MacOSX/environment.plist
  Info "set LDRAWDIR Completed."
  # Qt setup - MacOS
  QMAKE_EXEC=qmake
  # Set dependency profiler and nubmer of CPUs
  LDD_EXEC="otool -L"
  CPU_CORES=$(sysctl -n hw.ncpu)
else
  if [ -n "${MSYS2}" ]; then
    # Qt setup - MinGW
    if [ -f "${LP3D_QT_BIN}/qmake6.exe" ] ; then
      QMAKE_EXEC=${LP3D_QT_BIN}/qmake6.exe
    elif [ -f "${LP3D_QT_BIN}/qmake.exe" ] ; then
      QMAKE_EXEC=${LP3D_QT_BIN}/qmake.exe
    else
      QMAKE_EXEC=${MINGW_PREFIX}/bin/qmake.exe
    fi
  else
    # Qt setup - Linux
    export QT_SELECT=$(which qmake6 && echo qt6 || echo qt5)
    if [ -f "${LP3D_QT_BIN}/qmake6" ] ; then
      QMAKE_EXEC=${LP3D_QT_BIN}/qmake6
    elif [ -f "${LP3D_QT_BIN}/qmake" ] ; then
      QMAKE_EXEC=$LP3D_QT_BIN/qmake
    else
      if [ -x "/usr/bin/qmake6" ] ; then
        QMAKE_EXEC=/usr/bin/qmake6
      elif [ -x "/usr/bin/qmake-qt5" ] ; then
        QMAKE_EXEC=/usr/bin/qmake-qt5
      elif [ -d "$LP3D_QT5_BIN" ] ; then
        QMAKE_EXEC=$LP3D_QT5_BIN/qmake
      else
        QMAKE_EXEC=qmake
      fi
    fi
  fi
  # Set dependency profiler and nubmer of CPUs
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
Info "Target CPU...............[${TARGET_CPU}]"
Info "Number Of CPU Cores......[${CPU_CORES}]"

# Get Qt version
Info && ${QMAKE_EXEC} -v && Info
QMAKE_EXEC="${QMAKE_EXEC} -makefile"

# Backup ld_library_path
[ -n "${LD_LIBRARY_PATH}" ] && LP3D_LD_LIBRARY_PATH_SAVED=$LD_LIBRARY_PATH || :

# Initialize mesa build flag
MesaBuildAttempt=0

# Processor and linkier flags for building local libs
if [ "$get_local_libs" = 1 ]; then
  export PATH=$LP3D_LL_USR/bin:$PATH && \
  Info "PATH: $PATH"
  export LD_LIBRARY_PATH=$LP3D_LL_USR/bin:$LP3D_LL_USR/lib64:$LD_LIBRARY_PATH && \
  Info "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
  LP3D_LDFLAGS="-L$LP3D_LL_USR/lib64" && \
  Info "LP3D_LDFLAGS: $LP3D_LDFLAGS" && Info
  export Q_LDFLAGS="$LP3D_LDFLAGS"
fi

# Renderer versions
VER_LDGLITE=ldglite-1.3
VER_LDVIEW=ldview-4.6
VER_POVRAY=lpub3d_trace_cui-3.8
# Renderer paths
[ -n "${MSYS2}" ] && Extn=".exe" || :
LP3D_LDGLITE=${DIST_PKG_DIR}/${VER_LDGLITE}/bin/${TARGET_CPU_QMAKE}/ldglite${Extn}
LP3D_LDVIEW=${DIST_PKG_DIR}/${VER_LDVIEW}/bin/${TARGET_CPU_QMAKE}/ldview${Extn}
LP3D_POVRAY=${DIST_PKG_DIR}/${VER_POVRAY}/bin/${TARGET_CPU}/lpub3d_trace_cui${Extn}

# Install build dependencies for MacOS
if [ "$OS_NAME" = "Darwin" ]; then
  depsLog=${LP3D_LOG_PATH}/${ME}_${host}_deps_$OS_NAME.log
  Msg="Install $OS_NAME renderer build dependencies..."
  Info && Info $Msg && Info $Msg > $depsLog 2>&1
  Info "----------------------------------------------------"
  Info "Platform.................[macos]"
  Info "Using Sudo...............[No]"
  for buildDir in ldview povray; do
     case ${buildDir} in
     ldview)
       brewDeps="gl2ps libjpeg minizip"
       ;;
     povray)
       brewDeps="$brewDeps boost openexr sdl2 libtiff autoconf automake"
       if [ "${CI}" != "true" ]; then
         brewDeps="$brewDeps pkg-config"
       fi
       ;;
     esac
  done
  Info -n "Checking for X11 (xquartz) at /usr/X11..."
  if [[ -d "/usr/X11/lib" && -d "/usr/X11/include" ]]; then
    Info "X11 found."
    depsList="X11"
  else
    Msg="X11 not found."
    Info $Msg && Info $Msg > $depsLog 2>&1
    Msg "LPub3D_Trace(POVRay) will not build the XWindow display."
    Info $Msg && Info $Msg > $depsLog 2>&1
    if [ "${CI}" != "true" ]; then
      Info "  You can install xquartz using homebrew:"
      Info "  \$ brew cask list"
      Info "  \$ brew cask install xquartz"
      Info "  Note: elevated access password will be required."
    fi
    MACOS_POVRAY_NO_XWINDOW="true"
  fi
  if [ -n "$brewDeps" ]; then
    [ -n "$depsList" ] && depsList="$depsList $brewDeps" || :
    Info "Dependencies List........[${depsList} boost]"
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
  [ "$(echo $platform_ver | cut -d. -f2)" = 13 ] && MACOS_POVRAY_NO_OPTIMIZ="true" || :
fi

# List 'PLATFORM_*', 'build_*' and 'LP3D_*' environment variables
Info
if [ "$platform_id" != "arch" ]; then
  if [ "$OBS" = "true" ]; then
    Info "PLATFORM* environment variables:" && compgen -v | grep PLATFORM_ | while read line; do echo $line=${!line}; done
    Info "build* environment variables:" && compgen -v | grep build_ | while read line; do echo $line=${!line}; done
  fi
  Info "LP3D* environment variables:" && compgen -v | grep LP3D_ | while read line; do echo $line=${!line}; done
  Info
fi

# =======================================
# Main Loop
# =======================================

renderers=(ldglite ldview povray)
for buildDir in "${renderers[@]}"; do
  buildDirUpper="$(echo ${buildDir} | awk '{print toupper($0)}')"
  artefactVer="VER_${buildDirUpper}"
  artefactBinary="LP3D_${buildDirUpper}"
  buildLog=${LP3D_LOG_PATH}/${ME}_${host}_build_${buildDir}.log
  linesBefore=1
  case ${buildDir} in
  ldglite)
    curlCommand="${LP3D_GITHUB_URL}/ldglite/archive/master.tar.gz"
    checkString="LDGLite Output"
    linesAfter="2"
    buildCommand="BuildLDGLite"
    validSubDir="app"
    validExe="${validSubDir}/${BUILD_ARCH}/ldglite${Extn}"
    buildType="${LP3D_BUILD_CONFIG}"
    displayLogLines=10
    ;;
  ldview)
    curlCommand="${LP3D_GITHUB_URL}/ldview/archive/lpub3d-build.tar.gz"
    checkString="LDView.*LPub3D Edition Image Output"
    linesAfter="6"
    buildCommand="BuildLDView"
    validSubDir="OSMesa"
    if [[ -n "${MSYS2}" && "${LP3D_LDVIEW_CUI_OPT}" == "WGL" ]]; then
      validSubDir=
      validExe="${BUILD_ARCH}/ldview${Extn}"
    else
      [ "${LP3D_LDVIEW_CUI_OPT}" == "Qt" ] && validSubDir="QT" || :
      validExe="${validSubDir}/${BUILD_ARCH}/ldview${Extn}"
    fi
    buildType="${LP3D_BUILD_CONFIG}"
    displayLogLines=100
    ;;
  povray)
    curlCommand="${LP3D_GITHUB_URL}/povray/archive/lpub3d/raytracer-cui.tar.gz"
    checkString="Render Statistics"
    linesAfter="42"
    buildCommand="BuildPOVRay"
    validSubDir="unix"
    validExe="${validSubDir}/lpub3d_trace_cui${Extn}"
    buildType="${LP3D_BUILD_CONFIG}"
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
    if [[ -n "$build_osmesa" && -z "$get_local_libs" && "$MesaBuildAttempt" != 1 ]]; then
      BuildMesaLibs
      MesaBuildAttempt=1
      if [[ $? != 0 ]]; then
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
    # POVRay build on MSYS2 (MSVCRT) is currently KO so skip for now.
    if [[ -n "${MSYS2}" && "${buildDir}" = "povray" ]]; then
      Info && Info "$platform_pretty detected. LPub3D_Trace(${buildDir}) will not be built."
      continue
    fi
    # Check if build folder exist - donwload tarball and extract if no binary exists or to generate dependency list
    if [[ ! -f "${!artefactBinary}" || "$LP3D_NO_DEPS" != "true" ]]; then
      Info && Info "Setup ${!artefactVer} source files..."
      Info "----------------------------------------------------"
      if [ ! -d "${buildDir}/${validSubDir}" ]; then
        # Check if tarball archive exist...
        Info && Info "$(echo ${buildDir} | awk '{print toupper($0)}') build folder does not exist. Checking for tarball archive..."
        if [ ! -f ${buildDir}.tar.gz ]; then
          Info -n "$(echo ${buildDir} | awk '{print toupper($0)}') tarball ${buildDir}.tar.gz does not exist. Downloading..."
          (curl $CURL_OPTS ${curlCommand} -o ${buildDir}.tar.gz) >$l.out 2>&1 && rm $l.out
          [ -f $l.out ] && Info "Failed." && tail -20 $l.out || Info "Ok"
        fi
        [ -f ${buildDir}.tar.gz ] && ExtractArchive ${buildDir} ${validSubDir} || Info "ERROR - Failed to download ${buildDir}.tar.gz"
      else
        cd "${buildDir}" || :
      fi
    fi
    # Install build dependencies - even if binary exists...
    if [[ "$OS_NAME" != "Darwin" && "$OBS" != "true" ]]; then
      if [ "$LP3D_NO_DEPS" != "true" ]; then
        Msg="Install ${!artefactVer} build dependencies..."
        Info && Info $Msg && Info $Msg >> $buildLog 2>&1
        Info "----------------------------------------------------"
        InstallDependencies ${buildDir}
        sleep .5
      fi
      if [[ -n "$build_osmesa" && "$MesaBuildAttempt" != 1 ]]; then
        Msg="Install ${buildDir} Mesa build dependencies..."
        Info && Info $Msg && Info $Msg >> $buildLog 2>&1
        Info "----------------------------------------------------"
        BuildMesaLibs "sudo"
        MesaBuildAttempt=1
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
      fi
    fi
    Msg="Build ${buildDir} finished."
    Info && Info $Msg && Info && Info $Msg >> $buildLog 2>&1
  else
    Msg="Renderer artefact binary for ${!artefactVer} exists - build skipped."
    Info && Info $Msg && Info && Info $Msg >> $buildLog 2>&1
  fi
  cd "${WD}" || :
done
# Package renderers as a deliverable
[ "${LP3D_PACKAGE_RENDERERS}" = "true" ] && PackageRenderers || :

# Restore ld_library_path
[ -n "${LP3D_LD_LIBRARY_PATH_SAVED}" ] && export LD_LIBRARY_PATH=$LP3D_LD_LIBRARY_PATH_SAVED || :

exit 0
