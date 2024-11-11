#!/bin/bash
# Trevor SANDY
# Last Update: November 10, 2024
# Build and package LPub3D for macOS
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  ME="${ME} for (${LP3D_ARCH})"
  if [ "$BUILD_OPT" = "verify" ]; then
    echo "$ME Verification Finished!"
  elif [ "$BUILD_OPT" = "compile" ]; then
    echo "LPub3D Compile Finished!"
  elif [ "$BUILD_OPT" = "renderers" ]; then
    echo "LPub3D Renderers Build Finished!"
  else
    echo "$ME Finished!"
  fi
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# Fake realpath
realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH_="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH_"
}

CWD=`pwd`

# Format name - SOURCED if $1 is empty
ME="CreateDmg"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

# automatic logging
[ -z "${WRITE_LOG}" ] && WRITE_LOG=${WRITE_LOG:-true} || :
[ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
if [ "${WRITE_LOG}" = "true" ]; then
    f="${LP3D_LOG_PATH}/$ME"
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
    exec > >(tee -a ${LOG})
    exec 2> >(tee -a ${LOG} >&2)
fi

echo "Start $ME execution at $CWD..."

# Change these when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d}"
LP3D_ARCH="${LP3D_ARCH:-$(uname -m)}"
echo && echo "   LPUB3D BUILD ARCH......[${LP3D_ARCH}]"
echo "   LPUB3D SOURCE DIR......[$(realpath .)]"

if [ "$BUILD_OPT" = "verify" ]; then
  echo "   BUILD OPTION...........[verify only]"
elif [ "$BUILD_OPT" = "compile" ]; then
  echo "   BUILD OPTION...........[comple only]"
elif [ "$BUILD_OPT" = "renderers" ]; then
  echo "   BUILD OPTION...........[renderers only]"
else
  echo "   BUILD OPTION...........[build package]"
fi
if [ "$GITHUB" = "true" ]; then
  echo "   CPU CORES..............[$(echo $([ -n "${LP3D_CPU_CORES}" ] && echo ${LP3D_CPU_CORES} || echo $(nproc)))]"
fi

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

echo "   LOG FILE...............[$([ -n "${LOG}" ] && echo ${LOG} || echo "not writing log")]" && echo

# when running with Installer Qt, use this block...
if [ "${CI}" != "true"  ]; then
  # use this instance of Qt if exist - this entry is my dev machine, change for your environment accordingly
  if [ -d ~/Qt/IDE/5.15.0/clang_64 ]; then
    export PATH=~/Qt/IDE/5.15.0/clang_64:~/Qt/IDE/5.15.0/clang_64/bin:$PATH
  else
    echo "PATH not udpated with Qt location, could not find ${HOME}/Qt/IDE/5.15.0/clang_64"
    exit 1
  fi
  echo
  echo "LPub3D will uninstall all versions of Boost ignoring dependencies and install 1.60."
  echo "You can stop here if you do not want to uninstall your current version of Boost"
  echo "or if you prefer to personally configure the availability of Boost 1.60."
  read -n 1 -p "Enter c to continue Boost uninstall: " getoption
  if [ "$getoption" = "c" ] || [ "$getoption" = "C" ]; then
     echo  "  You selected to uninstall any current versions of Boost."
  else
     echo  "  You selected to stop the installation."
     exit 0
  fi
  echo
  echo "Enter d to download LPub3D source or any key to"
  echo "skip download and use existing source if available."
  read -n 1 -p "Do you want to continue with this option? : " getsource
else
  # Use this block for Homebrew Qt (script called from clone directory - e.g. lpub3d)
  # Set no-prompt getsource flag: 'c' = copy flag, 'd' = download flag
  getsource=c
  # move outside clone directory (lpub3d)/
  cd ../
fi

echo "-  create DMG build working directory $(realpath dmgbuild/)..."
if [ ! -d dmgbuild ]
then
  mkdir dmgbuild
fi

cd dmgbuild

if [ "$getsource" = "d" ] || [ "$getsource" = "D" ]
then
  echo "-  you selected download LPub3D source."
  echo "-  cloning ${LPUB3D}/ to $(realpath dmgbuild/)..."
  if [ -d ${LPUB3D} ]; then
    rm -rf ${LPUB3D}
  fi
  git clone https://github.com/trevorsandy/${LPUB3D}.git
elif [ "$getsource" = "c" ] || [ "$getsource" = "C" ] || [ ! -d ${LPUB3D} ]
then
  echo "-  copying ${LPUB3D}/ to $(realpath dmgbuild/)..."
  if [ ! -d ../${LPUB3D} ]; then
    echo "-  NOTICE - Could not find folder $(realpath ../${LPUB3D}/)"
    if [ -d ${LPUB3D} ]; then
       rm -rf ${LPUB3D}
    fi
    echo "-  cloning ${LPUB3D}/ to $(realpath dmgbuild/)..."
    git clone https://github.com/trevorsandy/${LPUB3D}.git
  else
    cp -rf ../${LPUB3D}/ ./${LPUB3D}/
  fi
else
  echo "-  ${LPUB3D}/ exist. skipping download"
fi

if [ -z "$LDRAWDIR" ]; then
  LDRAWDIR=${HOME}/Library/LDraw
fi

# set pwd before entering lpub3d root directory
export WD=$PWD
export OBS=false
export LPUB3D=${LPUB3D}

echo "-  source update_config_files.sh..." && echo

_PRO_FILE_PWD_=${WD}/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh
SOURCE_DIR=${LPUB3D}-${LP3D_VERSION}

case ${LP3D_ARCH} in
  "x86_64"|"aarch64"|"arm64")
    release="64bit_release" ;;
  *)
    release="32bit_release" ;;
esac

echo "-  execute CreateRenderers from $(realpath ${LPUB3D}/)..."

cd ${LPUB3D}

cat <<EOF >rendererVars.sh
export WD=${WD}
export OBS=false
export LPUB3D=${LPUB3D}
export LDRAWDIR=${LDRAWDIR}
export LP3D_LOG_PATH=${LP3D_LOG_PATH}
export LP3D_3RD_DIST_DIR=${LP3D_3RD_DIST_DIR}
EOF

chmod +x builds/utilities/CreateRenderers.sh && \
./builds/utilities/CreateRenderers.sh

DIST_DIR="$(cd ../ && echo "$PWD/lpub3d_macos_3rdparty")"

# POVRay configure uses aarch64 architecture value on arm64 Mac build
[ "${LP3D_ARCH}" = "arm64" ] && LP3D_RAY_ARCH="aarch64" || LP3D_RAY_ARCH=${LP3D_ARCH}

# Check if renderers exist or were successfully built
BUILD_RENDERERS=ok
LDGLITE_PATH="${DIST_DIR}/LDGLite-1.3/bin/${LP3D_ARCH}"
LDVIEW_PATH="${DIST_DIR}/LDView-4.5/bin/${LP3D_ARCH}"
POVRAY_PATH="${DIST_DIR}/lpub3d_trace_cui-3.8/bin/${LP3D_RAY_ARCH}"
if [ ! -f "${LDGLITE_PATH}/LDGLite" ]; then
  BUILD_RENDERERS=ko && echo "ERROR - LDGLite not found at ${LDGLITE_PATH}/"
fi
if [ ! -f "${LDVIEW_PATH}/LDView" ]; then
  BUILD_RENDERERS=ko && echo "ERROR - LDView not found at ${LDVIEW_PATH}/"
fi
if [ ! -f "${POVRAY_PATH}/lpub3d_trace_cui" ]; then
  BUILD_RENDERERS=ko && echo "ERROR - POVRay not found at ${POVRAY_PATH}/"
fi
if [ "${BUILD_RENDERERS}" = "ko" ]; then
  echo "ERROR - all renderers were not successfully built, ${LPUB3D} build cannot continue."
  exit 1
fi

# Stop here if we are only building renderers
if [ "$BUILD_OPT" = "renderers" ]; then
  exit 0
fi

# Copy LDraw archive libraries to mainApp/extras
if [ ! -f "mainApp/extras/complete.zip" ]
then
  if [ -f "${DIST_DIR}/complete.zip" ]
  then
    echo "-  copy ldraw official library archive from ${DIST_DIR}/ to $(realpath mainApp/extras/)..."
    cp -f "${DIST_DIR}/complete.zip" "mainApp/extras/complete.zip"
  else
    echo "-  download ldraw official library archive to $(realpath mainApp/extras/)..."
    curl $curlopts https://library.ldraw.org/library/updates/complete.zip -o mainApp/extras/complete.zip
  fi
else
  echo "-  ldraw official library exist. skipping download"
fi
if [ ! -f "mainApp/extras/lpub3dldrawunf.zip" ]
then
  if [ -f "${DIST_DIR}/lpub3dldrawunf.zip" ]
  then
    echo "-  copy unofficial library archive from ${DIST_DIR}/ to $(realpath mainApp/extras/)..."
    cp -f "${DIST_DIR}/lpub3dldrawunf.zip" "mainApp/extras/lpub3dldrawunf.zip"
  else
    echo "-  download ldraw unofficial library archive to $(realpath mainApp/extras/)..."
    curl $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/lpub3dldrawunf.zip -o mainApp/extras/lpub3dldrawunf.zip
  fi
else
  echo "-  ldraw unofficial library exist. skipping download"
fi
if [ ! -f "mainApp/extras/tenteparts.zip" ]
then
  if [ -f "${DIST_DIR}/tenteparts.zip" ]
  then
    echo "-  copy Tente library archive from ${DIST_DIR}/ to $(realpath mainApp/extras/)..."
    cp -f "${DIST_DIR}/tenteparts.zip" "mainApp/extras/tenteparts.zip"
  else
    echo "-  download ldraw Tente library archive to $(realpath mainApp/extras/)..."
    curl $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/tenteparts.zip -o mainApp/extras/tenteparts.zip
  fi
else
  echo "-  TENTE library exist. skipping download"
fi

if [ ! -f "mainApp/extras/vexiqparts.zip" ]
then
  if [ -f "${DIST_DIR}/vexiqparts.zip" ]
  then
    echo "-  copy VEXIQ library archive from ${DIST_DIR}/ to $(realpath mainApp/extras/)..."
    cp -f "${DIST_DIR}/vexiqparts.zip" "mainApp/extras/vexiqparts.zip"
  else
    echo "-  download ldraw VEXIQ library archive to $(realpath mainApp/extras/)..."
    curl $curlopts https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/vexiqparts.zip -o mainApp/extras/vexiqparts.zip
  fi
else
  echo "-  VEXIQ library exist. skipping download"
fi

echo && echo "-  configure and build source from $(realpath .)..."
#qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 /usr/bin/make qmake_all
echo && qmake -v && echo
qmake CONFIG+=x86_64 CONFIG+=release CONFIG+=sdk_no_version_check CONFIG+=build_check CONFIG-=debug_and_release CONFIG+=dmg
if [ -n "${LP3D_CPU_CORES}" ]; then
  /usr/bin/make -j${LP3D_CPU_CORES}
else
  /usr/bin/make -j$(nproc)
fi

# Check if build is OK or stop and return error.
if [ ! -f "mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D" ]; then
  echo "ERROR - build executable at $(realpath mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D) not found."
  exit 1
else
  # run otool -L on LPub3D.app
  echo && echo "otool -L check LPub3D.app/Contents/MacOS/LPub3D..." && \
  otool -L mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D 2>/dev/null || \
  echo "ERROR - oTool check failed for $(realpath mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D)"
  # Stop here if we are only compiling
  if [ "$BUILD_OPT" = "compile" ]; then
    exit 0
  fi
fi

# create dmg environment - begin #
#
cd builds/macx

echo "- copy ${LPUB3D} bundle components to $(realpath .)..."
cp -rf ../../mainApp/$release/LPub3D.app .

echo "- install LDrawIni and QuaZIP library links..."
/usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/Libs/libQuaZIP.1.dylib

echo "- change LDrawIni and QuaZIP mapping to LPub3D..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libQuaZIP.1.dylib @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "- bundle LPub3D to add Qt framework..."
macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "- change LDrawIni and QuaZIP library dependency mapping to Qt framework..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libQuaZIP.1.dylib @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "- reapply the code signature..."
/usr/bin/codesign -f --deep -s - LPub3D.app

LPUB3D_EXE=LPub3D.app/Contents/MacOS/LPub3D
if [ -n "$LP3D_SKIP_BUILD_CHECK" ]; then
  echo "- skipping ${LPUB3D_EXE} build check."
else
  echo "- build checks..."
  # Check if exe exist - here we use the executable name
  if [ -f "${LPUB3D_EXE}" ]; then
    echo "- Build package: $PWD/${LPUB3D_EXE}"
    # Check commands
    SOURCE_DIR=../..
    echo "- build check SOURCE_DIR is $(realpath ${SOURCE_DIR})..."
    source ${SOURCE_DIR}/builds/check/build_checks.sh
    # Stop here if we are only verifying
    if [ "$BUILD_OPT" = "verify" ]; then
      exit 0
    fi
  else
    echo "- ERROR - build-check failed. $(realpath ${LPUB3D_EXE}) not found."
  fi
fi

echo "- setup dmg source dir $(realpath DMGSRC/)..."
if [ -d DMGSRC ]
then
  rm -f -R DMGSRC
fi
mkdir DMGSRC

echo "- move LPub3D.app to $(realpath DMGSRC/)..."
mv -f LPub3D.app DMGSRC/LPub3D.app

echo "- setup dmg output directory $(realpath ../../../DMGS/)..."
DMGDIR=../../../DMGS
if [ -d ${DMGDIR} ]
then
  rm -f -R ${DMGDIR}
fi
mkdir -p ${DMGDIR} && \
echo "- created dmg output directory $(realpath $DMGDIR)"

# pos: builds/macx
echo "- generate README file and dmg make script..."
cat <<EOF >README
Thank you for installing LPub3D v${LP3D_APP_VERSION} for macOS.

Drag the LPub3D Application icon to the Applications folder.

After installation, remove the mounted LPub3D disk image by dragging it to the Trash.

Required LPub3D libraries for macOS distribution.
Library versions for LPub3D built from source may differ.
========================
LDView:

- XQuartz version 11.0 (2.7.11) or above (needed for OSMesa)
  https://www.xquartz.org

- LibPNG version 1.6.37 or above
  http://www.libpng.org

- GL2PS version 1.4.0 or above
  http://geuz.org/gl2ps

- LibJPEG version 9c or above
  http://www.ijg.org

- TinyXML version 2.6.2 or above
  http://www.grinninglizard.com/tinyxml/

- MiniZIP version 1.2.11 or above
  http://www.winimage.com/zLibDll/minizip.html

POVRay:

- XQuartz version 11.0 (2.7.11)  or above (needed for X11)
  https://www.xquartz.org

- LibTIFF version 4.0.10 or above
  http://www.libtiff.org

- OpenEXR greater than version 2.3.0 or above
  http://www.openexr.com

- SDL2 version 2.0.10 or above (for display preview)
  http://www.libsdl.org

Homebrew
======================================
LPub3D and its renderers are compiled for both the Apple silicon arm64 and Intel x86_64 processors.

Running LPub3D x86_64 on an Apple silicon PC will require Rosetta which, if not already
installed on your PC will, you will be prompted to install it on your attempt to run LPub3D.
If you wish to install Rosetta from the command line, the command is:

- \$ /usr/sbin/softwareupdate --install-rosetta agree-to-license (root permission required)

Install brew (if not already installed)
======================================
For an Apple Intel processor:
- \$ /usr/bin/ruby -e "\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

For an Apple silicon arm64 processor:
- \$ /bin/bash -c "\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

For an x86_64 brew installation on an Apple silicon arm64 processor:
- \$ arch -x86_64 zsh
- \$ cd /usr/local && mkdir homebrew
- \$ curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew

Install libraries
=================
For an Apple silicon or Apple Intel processor:
- \$ brew update
- \$ brew reinstall libpng tinyxml gl2ps libjpeg minizip openexr sdl2 libtiff
- \$ brew install --cask xquartz

For an x86_64 brew installation on an Apple silicon arm64 processor:
- \$ arch -x86_64 /usr/local/homebrew/bin/brew reinstall libpng tinyxml gl2ps libjpeg minizip openexr sdl2 libtiff
- \$ arch -x86_64 /usr/local/homebrew/bin/brew install --cask xquartz

Optional - Check installed library (e.g. libpng)
============================================
- \$ otool -L \$(brew list libpng | grep dylib\$)

Object Tool (otool) output:
  /usr/local/Cellar/libpng/1.6.35/lib/libpng.dylib:
  /usr/local/opt/libpng/lib/libpng16.16.dylib (compatibility version 52.0.0, current version 52.0.0)
  /usr/lib/libz.1.dylib (compatibility version 1.0.0, current version 1.2.11)
  /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)

LPub3D Library Check Note: On startup LPub3D will test for the /opt/homebrew/bin/brew binary.
If found, the library check will use the Apple silicon path prefix:
- HomebrewLibPathPrefix: /opt/homebrew/opt.
...otherwise, it will use the Apple intel path prefix.
- HomebrewLibPathPrefix: /usr/local/opt.

Additionally, LPub3D will check Homebrew x86_64 libraries using PATH entries:
- LibraryCheckPathInsert: PATH=/usr/local/Homebrew/bin:/opt/local/bin:/usr/local/bin
...and Homebrew arm64 libraries using PATH entries:
- LibraryCheckPathInsert: PATH=/opt/homebrew/bin:/opt/homebrew/sbin
Of course, the assumption is you are running the appropriate macOS distribution for your processor.

If you choose to run an Intel distribution of LPub3D on an Apple silicon PC, running Rosetta
etc..., you are advised to configure the LPub3D plist with the appropriate Intel Homebrew path.
Or, if you choose to place your Homebrew binaries and libraries in alternate locations,
You can configure your personalized paths in the LPub3D plist at:
- \$HOME/Library/Preferences/com.lpub3d-software.LPub3D.plist.

The Homebrew plist keys are:
- HomebrewLibPathPrefix - the path prefix LPub3D will use to locate the Homebrew libraries.
- LibraryCheckPathInsert - the PATH entries needed to help brew run the info command

Cheers,
EOF

echo "- copy ${LPUB3D} package assets to to $(realpath .)..."
cp -f ../utilities/icons/setup.icns .
cp -f ../utilities/icons/lpub3dbkg.png .
cp -f ../../mainApp/docs/COPYING_BRIEF .COPYING

echo "- set create-dmg build scrpt permissions..."
chmod +x ../utilities/dmg-utils/create-dmg

echo "- copy README to Resources/README_macOS.txt..."
cp -f README DMGSRC/LPub3D.app/Contents/Resources/README_macOS.txt

echo "- generate makedmg script..."
LP3D_DMG="LPub3D-${LP3D_APP_VERSION_LONG}-${LP3D_ARCH}-macos.dmg"
cat <<EOF >makedmg
#!/bin/bash
../utilities/dmg-utils/create-dmg \\
--volname "LPub3D-Installer" \\
--volicon "setup.icns" \\
--background "lpub3dbkg.png" \\
--icon-size 90 \\
--text-size 10 \\
--window-pos 200 120 \\
--window-size 640 480 \\
--icon LPub3D.app 192 344 \\
--hide-extension LPub3D.app \\
--add-file Readme README 512 128 \\
--app-drop-link 448 344 \\
--eula .COPYING \\
"${DMGDIR}/${LP3D_DMG}" \\
DMGSRC/
EOF

echo "- create LPub3D dmg package in $(realpath $DMGDIR/)..."
[[ -f LPub3D-Installer.dmg ]] && rm LPub3D-Installer.dmg
if [ -d DMGSRC/LPub3D.app ]; then
   chmod +x makedmg && ./makedmg
else
  echo "- Could not find LPub3D.app at $(realpath DMGSRC/)"
  echo "- $ME Failed."
  exit 1
fi

if [ -f "${DMGDIR}/${LP3D_DMG}" ]; then
echo "      Distribution package.: ${LP3D_DMG}"
  echo "      Package path.........: $PWD/${LP3D_DMG}"
  echo "- cleanup..."
  rm -f -R DMGSRC
  rm -f lpub3d.icns lpub3dbkg.png README .COPYING makedmg
else
echo "- ${DMGDIR}/${LP3D_DMG} was not found."
  echo "- $ME Failed."
fi

exit 0
