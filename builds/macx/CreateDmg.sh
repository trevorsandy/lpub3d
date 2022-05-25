#!/bin/bash
# Trevor SANDY
# Last Update: August 06, 2021
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

# Format the log name - SOURCED if $1 is empty
WRITE_LOG=${WRITE_LOG:-true}
ME="CreateDmg"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced

CWD=`pwd`

# logging stuff
if [ "${WRITE_LOG}" = "true" ]; then
    [ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$CWD || :
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
echo && echo "   LPUB3D SOURCE DIR......[$(realpath .)]"

if [ "$BUILD_OPT" = "verify" ]; then
  echo "   BUILD OPTION...........[verify only]"
elif [ "$BUILD_OPT" = "compile" ]; then
  echo "   BUILD OPTION...........[comple only]"
elif [ "$BUILD_OPT" = "renderers" ]; then
  echo "   BUILD OPTION...........[renderers only]"
else
  echo "   BUILD OPTION...........[build package]"
fi

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

echo "   LOG FILE...............[${LOG}]" && echo

# when running locally, use this block...
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
  # For Travis CI, use this block (script called from clone directory - lpub3d)
  # getsource = downloaded source variable; 'c' = copy flag, 'd' = download flag
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

echo "-  source update_config_files.sh..." && echo

_PRO_FILE_PWD_=$PWD/${LPUB3D}/mainApp
source ${LPUB3D}/builds/utilities/update-config-files.sh
SOURCE_DIR=${LPUB3D}-${LP3D_VERSION}

# set pwd before entering lpub3d root directory
export OBS=false; export WD=$PWD; export LPUB3D=${LPUB3D}
if [ -z "$LDRAWDIR" ]; then
  LDRAWDIR=${HOME}/ldraw
fi
export LDRAWDIR=${LDRAWDIR}

DistArch=$(uname -m)
if [ "${DistArch}" = "x86_64" ]; then release="64bit_release"; else release="32bit_release"; fi

echo "-  execute CreateRenderers from $(realpath ${LPUB3D}/)..."
cd ${LPUB3D}

chmod +x builds/utilities/CreateRenderers.sh
./builds/utilities/CreateRenderers.sh

# Check if renderers exist or were successfully built
if [ ! -f "../lpub3d_macos_3rdparty/lpub3d_trace_cui-3.8/bin/$DistArch/lpub3d_trace_cui" ] || \
   [ ! -f "../lpub3d_macos_3rdparty/LDView-4.4/bin/$DistArch/LDView" ] || \
   [ ! -f "../lpub3d_macos_3rdparty/LDGLite-1.3/bin/$DistArch/LDGLite" ]
then
  echo "ERROR - all renderers were not accounted for, the build cannot continue."
  exit 1
fi

# Stop here if we are only building renderers
if [ "$BUILD_OPT" = "renderers" ]; then
  exit 0
fi

if [ ! -f "mainApp/extras/complete.zip" ]
then
  DIST_DIR="../lpub3d_macos_3rdparty"
  if [ -f "${DIST_DIR}/complete.zip" ]
  then
    echo "-  copy ldraw official library archive from ${DIST_DIR}/ to $(realpath mainApp/extras/)..."
    cp -f "${DIST_DIR}/complete.zip" "mainApp/extras/complete.zip"
  else
    echo "-  download ldraw official library archive to $(realpath mainApp/extras/)..."
    curl $curlopts http://www.ldraw.org/library/updates/complete.zip -o mainApp/extras/complete.zip
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
    curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o mainApp/extras/lpub3dldrawunf.zip
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
qmake CONFIG+=x86_64 CONFIG+=release CONFIG+=build_check CONFIG-=debug_and_release CONFIG+=dmg
/usr/bin/make

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
cp -f ../utilities/icons/setup.icns .
cp -f ../utilities/icons/setup.png .
cp -f ../utilities/icons/lpub3dbkg.png .
cp -f ../../mainApp/docs/COPYING_BRIEF .COPYING

echo "- set scrpt permissions..."
chmod +x ../utilities/create-dmg
chmod +x ../utilities/dmg-utils/licenseDMG.py

echo "- install library links..."
/usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/Libs/libQuaZIP.1.dylib

echo "- change mapping to LPub3D..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libQuaZIP.1.dylib @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "- bundle LPub3D..."
macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "- change library dependency mapping..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libQuaZIP.1.dylib @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "- build checks..."
# Check if exe exist - here we use the executable name
LPUB3D_EXE=LPub3D.app/Contents/MacOS/LPub3D
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
mkdir -p ${DMGDIR}

# pos: builds/macx
echo "- generate README file and dmg make script..."
cat <<EOF >README
Thank you for installing LPub3D v${LP3D_APP_VERSION} for macOS".

Drag the LPub3D Application icon to the Applications folder.

After installation, remove the mounted LPub3D disk image by dragging it to the Trash.

Required LPub3D libraries for built distribution.
Library versions for LPub3D built from source may differ.
========================
LDView:

- XQuartz 11.0 (for OSMesa)
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

- XQuartz 11.0 (for X11)
  https://www.xquartz.org

- LibTIFF version 4.0.10 or above
  http://www.libtiff.org

- OpenEXR greater than version 2.3.0 and less then version 3.0.0
  http://www.openexr.com

- SDL2 version 2.0.10 or above (for display preview)
  http://www.libsdl.org

Install brew (if not already installed)
======================================
- \$ /usr/bin/ruby -e "\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

Install libraries
=================
- \$ brew update
- \$ brew reinstall libpng tinyxml gl2ps libjpeg minizip openexr sdl2 libtiff
- Download XQuartz at https://dl.bintray.com/xquartz/downloads/XQuartz-2.7.11.dmg
- Double click the XQuartz-2.7.11.dmg and follow the dialogue instructions.

Optional - Check installed lirary (e.g. libpng)
============================================
- \$ otool -L $(brew list libpng | grep dylib$)
    /usr/local/Cellar/libpng/1.6.35/lib/libpng.dylib:
        /usr/local/opt/libpng/lib/libpng16.16.dylib (compatibility version 52.0.0, current version 52.0.0)
        /usr/lib/libz.1.dylib (compatibility version 1.0.0, current version 1.2.11)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)

Cheers,
EOF

echo "- copy README to Resources/README_macOS.txt..."
cp -f README DMGSRC/LPub3D.app/Contents/Resources/README_macOS.txt

echo "- generate makedmg script..."
cat <<EOF >makedmg
#!/bin/bash
../utilities/create-dmg \\
--volname "LPub3D-Installer" \\
--volicon "setup.icns" \\
--dmgicon "setup.png" \\
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
--skip-jenkins \\
"${DMGDIR}/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg" \\
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

if [ -f "${DMGDIR}/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg" ]; then
  echo "      Distribution package.: LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg"
  echo "      Package path.........: $PWD/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg"
  echo "- cleanup..."
  rm -f -R DMGSRC
  rm -f lpub3d.icns lpub3dbkg.png setup.png README .COPYING makedmg
else
  echo "- ${DMGDIR}/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg was not found."
  echo "- $ME Failed."
fi

exit 0
