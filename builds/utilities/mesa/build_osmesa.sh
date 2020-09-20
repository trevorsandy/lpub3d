#!/bin/bash
#
# Build all libOSMesa and libGLU libraries - short
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: March 06, 2019
#  Copyright (c) 2018 - 2020 by Trevor SANDY
#
# Useage: env WD=$PWD [COPY_CONFIG=1] ./lpub3d/builds/utilities/mesa/buildosmesa.sh
# Note: If WD is not defined,
#

# capture elapsed time - reset BASH time counter
SECONDS=0

# configuration options:
# specify the osmesa verion to build
mesaversion="${OSMESA_VERSION:-18.3.5}"
# specify if gallium dirver not available (e.g. RHEL on OBS does not support llvm so gallium driver not available)
nogallium="${NO_GALLIUM:-0}"
# specify llvm-config path if different from system default
llvm_config="${LLVM_CONFIG:-/usr/bin/llvm-config}"
# specify the libglu version
gluversion="${GLU_VERSION:-9.0.0}"
# specify the number of job processes
mkjobs="${MKJOBS:-4}"
# tell turl to follow links and continue after abnormal end if supported
curlopts="-L -C -"
# specify the build output path
osmesaprefix="${OSMESA_PREFIX:-$WD/lpub3d_linux_3rdparty/mesa}"
# specify if to remove existing build and build new
cleanbuild="${CLEAN:-0}"
# specify if to overwrite the existing osmesa-config - this file is copied during the build process
config_copy="${COPY_CONFIG:-0}"
# /usr
local_usr_path="${LOCAL_USR_PATH:-}"
# building on Open Build Service
#using_obs="${OBS:-false}"
# compiler flags
CC="gcc"
CXX="g++"
CFLAGS="-O2"
CXXFLAGS="-O2 -std=c++11"

# grab te script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# get the script location
ScriptDir=$(dirname "$0")

# set the osmesa-config default location
osmesaConfDir=$(cd $ScriptDir && echo $PWD)

# Get platform
OS_NAME=`uname`
if [ "$OS_NAME" = "Darwin" ]; then
  PLATFORM=$(echo `sw_vers -productName`_`sw_vers -productVersion`)
else
  PLATFORM=$(. /etc/os-release && if test "${NAME}" != "" && test "${VERSION_ID}" != ""; then echo "${NAME}_${VERSION_ID}"; else echo `uname`; fi)
fi

if [ ! "$OBS" = "true" ]; then
  # logging stuff
  # increment log file name
  f="$PWD/${ME}_${PLATFORM}"
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

# Functions
Info () {
  echo "-osmesa- ${*}" >&2
}

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
    WD="$(cd ../ && pwd)"
  Info "WARNING - 'WD' environment varialbe not specified. Using $WD"
fi

Info
Info "Working Directory........[${WD}]"
Info "Script Directory.........[${ScriptDir}]"
Info "OSMesa-config Directory..[$osmesaConfDir]"
if [ " $nogallium" = 1 ]; then
  Info "LLVM not needed..........[Gallium driver not available]"
else
  Info "LLVM-Config Path.........[${llvm_config}]"
fi
Info "Install Prefix...........[${osmesaprefix}]"
Info "OSMesa Version...........[${mesaversion}]"
Info "GLU Version..............[${gluversion}]"

Info && Info "----------------------------------------------------"
Info "Building OSMesa..."

cd $WD
if [[ -d "mesa-${mesaversion}" && "${cleanbuild}" = 1 ]]; then
    Info "cleanup old mesa-$mesaversion..."
    rm -rf "mesa-$mesaversion"
    if [ -d "${osmesaprefix}" ]; then
        rm -rf "${osmesaprefix}"
    fi
fi

#check for llvm-config - and process OBS alternative config (e.g. no gallium)
if [ ! -f "${llvm_config}" ]; then
  if [[ "$OBS" = "true" && !"${PLATFORM}" = "arch" ]]; then
    if [ ! "$nogallium" = 1 ]; then
        Info "ERROR - llmv-config not found. $ME will terminate"
        exit 1
    fi
  else
    if [ !"${PLATFORM}" = "arch" ]; then
        Info "llmv-config not found, (re)installing Mesa build dependencies..."
        sudo dnf builddep -y mesa
    fi
  fi
else
  Info "Found llvm_config at $llvm_config"
fi

# sourcepath="${SOURCE_PATH:-projects/Working/Docker-output}"
if [ ! -f "mesa-${mesaversion}.tar.gz" ]; then
  if [ ! "$OBS" = "true" ]; then
    Info "downloading Mesa ${mesaversion}..."
    curl $curlopts -O "https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/mesa-${mesaversion}.tar.gz"
  else
    Info "ERROR - archive file mesa-${mesaversion}.tar.gz was not found. $ME will terminate."
    exit 1
  fi
fi

if [ ! -d "mesa-${mesaversion}" ]; then
    Info "extracting Mesa..."
    tar zxf mesa-${mesaversion}.tar.gz
fi

if [ ! -d "${osmesaprefix}" ]; then
    Info "create install prefix..."
    mkdir -p "${osmesaprefix}"
fi

cd mesa-${mesaversion}

# OSMesa configuration options
confopts="\
--disable-dependency-tracking \
--enable-static \
--disable-shared \
--enable-texture-float \
--disable-gles1 \
--disable-gles2 \
--disable-dri \
--disable-dri3 \
--disable-glx \
--disable-glx-tls \
--disable-egl \
--disable-gbm \
--disable-xvmc \
--disable-vdpau \
--disable-va \
--disable-opencl \
--disable-shared-glapi \
--disable-driglx-direct \
--with-dri-drivers= \
--with-platforms= \
--with-osmesa-bits=32 \
"
if [ "${mesaversion}" = "18.3.5" ]; then
  confopts="\
  $confopts \
  --disable-omx-bellagio \
  "
else
  confopts="\
  $confopts \
  --disable-omx \
  "
fi
if [ "$nogallium" = 1 ]; then
  confopts="\
  $confopts \
  --disable-gallium-osmesa \
  --disable-llvm \
  --enable-osmesa \
  "
else
  confopts="\
  $confopts \
  --disable-osmesa \
  --enable-gallium-osmesa \
  --enable-llvm=yes \
  --disable-llvm-shared-libs \
  --with-gallium-drivers=swrast \
  "
fi
confopts="\
$confopts \
--prefix=${osmesaprefix} \
"
Info "Using confops: $confopts"
if [ ! -f "$osmesaprefix/lib/libOSMesa32.a" ]; then
    # configure command
    env PKG_CONFIG_PATH="$osmesaprefix/lib/pkgconfig:$PKG_CONFIG_PATH" \
    ./configure ${confopts} CC="$CC" CFLAGS="$CFLAGS" CXX="$CXX" CXXFLAGS="$CXXFLAGS"

    # build command
    Info && make -j${mkjobs}

    # install command [sudo is not needed with user install prefix]
    Info "installing OSMesa..."
    make install
else
    Info "library OSMesa32 exist - build skipped."
fi

# copy config file
if [[ $config_copy -eq 1 || ! -f "${osmesaprefix}/osmesa-config" ]]; then
    cp -f "${ScriptDir}/osmesa-config" "${osmesaprefix}"
    if [ -f "${osmesaprefix}/osmesa-config" ]; then
        Info "osmesa-config copied to ${osmesaprefix}"
        Info "setting permissions..."
        chmod +x "${osmesaprefix}/osmesa-config"
    else
        Info "ERROR - osmesa-config was not copied to ${osmesaprefix}"
    fi
elif  [ -f "${osmesaprefix}/osmesa-config" ]; then
    Info "osmesa-config exist - copy skipped."
fi

# update version in config file
if  [ -f "${osmesaprefix}/osmesa-config" ]; then
    sed '/    --version)/{n;s/.*/      echo '"${mesaversion}"'/}' -i ${osmesaprefix}/osmesa-config
    Info "osmesa-config version updated"
    sed -n '42,43p;44q' ${osmesaprefix}/osmesa-config
else
    Info "osmesa-config was not found - version update failed."
fi

Info && Info "----------------------------------------------------"
Info "Building GLU..."

cd $WD
if [[ -d "glu-$gluversion" && ${cleanbuild} = 1 ]]; then
    Info "cleanup old glu-$gluversion..."
    rm -rf "glu-$gluversion"
    if [ -d "${osmesaprefix}" ]; then
        rm -rf "${osmesaprefix}"
    fi
fi

if [ ! -f "glu-${gluversion}.tar.bz2" ]; then
  if [ ! "$OBS" = "true" ]; then
    Info "* downloading GLU ${gluversion}..."
    curl $curlopts -O "https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/glu-${gluversion}.tar.bz2"
  else
    Info "ERROR - archive file glu-${gluversion}.tar.bz2 was not found."
  fi
fi

if [ ! -d "glu-${gluversion}" ]; then
    Info "extracting GLU..."
    tar jxf glu-${gluversion}.tar.bz2
fi

if [ ! -d "${osmesaprefix}" ]; then
    Info "create install prefix..."
    mkdir -p "${osmesaprefix}"
fi

cd glu-${gluversion}

# GLU configuration options
confopts="\
--disable-dependency-tracking \
--enable-static \
--disable-shared \
--enable-osmesa \
--prefix=$osmesaprefix \
"
if [ ! -f "$osmesaprefix/lib/libGLU.a" ]; then
    # configure command
    env PKG_CONFIG_PATH="$osmesaprefix/lib/pkgconfig:$PKG_CONFIG_PATH" \
    ./configure ${confopts} CC="$CC" CFLAGS="$CFLAGS" CXX="$CXX" CXXFLAGS="$CXXFLAGS"

    # build command
    Info && make -j${mkjobs}

    # install command [sudo is not needed with user install prefix]
    Info "installing GLU..."
    make install
else
    Info "library GLU exist - build skipped."
fi
cd $WD

# elapsed execution time
ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
Info
Info "$ME Finsihed!"
Info "$ELAPSED"
Info "----------------------------------------------------"
