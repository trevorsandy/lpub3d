#!/bin/bash
# Trevor SANDY
# Last Update March 20, 2017
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

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

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`

echo "Start $ME execution at $CWD..."

# Change these when you change the LPub3D root directory (e.g. if using a different root folder when testing)
LPUB3D="${LPUB3D:-lpub3d}"
echo && echo "   LPUB3D SOURCE DIR......[$(realpath .)]"

if [ "$BUILD_OPT" = "compile" ]; then
  echo "   BUILD OPTION...........[comple only]"
else
  echo "   BUILD OPTION...........[build package]"
fi

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# logging stuff
# increment log file name
f="${CWD}/$ME"
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
echo "   LOG FILE...............[${LOG}]" && echo

# when running locally, use this block...
if [ "${TRAVIS}" != "true"  ]; then
  # use this instance of Qt if exist - this entry is my dev machine, change for your environment accordingly
  if [ "${TRAVIS}" != "true" ]; then
    if [ -d ~/Qt/IDE/5.10.1/clang_64 ]; then
      export PATH=~/Qt/IDE/5.10.1/clang_64:~/Qt/IDE/5.10.1/clang_64/bin:$PATH
    else
      echo "PATH not udpated with Qt location, could not find ${HOME}/Qt/IDE/5.9/clang_64"
    fi
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
export OBS=false; export WD=$PWD; export LPUB3D=${LPUB3D}; export LDRAWDIR=${HOME}/ldraw

echo "-  execute CreateRenderers from $(realpath ${LPUB3D}/)..."
cd ${LPUB3D}

chmod +x builds/utilities/CreateRenderers.sh
./builds/utilities/CreateRenderers.sh

if [ ! -f "mainApp/extras/complete.zip" ]
then
  if [ -f "${HOME}/Library/complete.zip" ]
  then
    echo "-  copy ldraw official library archive from ${HOME}/Library/ to $(realpath mainApp/extras/)..."
    cp -f "${HOME}/Library/complete.zip" "mainApp/extras/complete.zip"
  else
    echo "-  download ldraw official library archive to $(realpath mainApp/extras/)..."
    curl $curlopts http://www.ldraw.org/library/updates/complete.zip -o mainApp/extras/complete.zip
  fi
else
  echo "-  ldraw official library exist. skipping download"
fi
if [ ! -f "mainApp/extras/lpub3dldrawunf.zip" ]
then
  echo "-  download ldraw unofficial library archive to $(realpath mainApp/extras/)..."
  curl $curlopts http://www.ldraw.org/library/unofficial/ldrawunf.zip -o mainApp/extras/lpub3dldrawunf.zip
else
  echo "-  ldraw unofficial library exist. skipping download"
fi

echo && echo "-  configure and build source from $(realpath .)..."
#qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 /usr/bin/make qmake_all
echo && qmake -v && echo
qmake CONFIG+=x86_64 CONFIG+=release CONFIG+=build_check CONFIG-=debug_and_release CONFIG+=dmg
/usr/bin/make

# Check if build is OK or stop and return error.
if test $(uname -m) = x86_64; then release="64bit_release"; else release="32bit_release"; fi
if [ ! -d "mainApp/$release/LPub3D.app" ]; then
  echo "ERROR - build output at $(realpath mainApp/$release/LPub3D.app/) not found."
  ElapsedTime
  exit 1
# Stop here if we are only compiling
elif [ "$BUILD_OPT" = "compile" ]; then
  ElapsedTime
  exit 0
else
  # run otool -L on LPub3D.app
  echo && echo "otool -L check LPub3D.app/Contents/MacOS/LPub3D..." && \
  otool -L mainApp/$release/LPub3D.app/Contents/MacOS/LPub3D 2>/dev/null || \
  echo "ERROR - otool -L check LPub3D.app/Contents/MacOS/LPub3D - failed."
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
chmod +x ../utilities/dmg-utils/dmg-license.py

echo "- install library links..."
/usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Libs/libQuaZIP.0.dylib

echo "- change mapping to LPub3D..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "- bundle LPub3D..."
macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "- change library dependency mapping..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "- build check..."
LPub3D.app/Contents/MacOS/LPub3D -foo && echo "- build check completed." && echo

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
Thank you for installing LPub3D v${APP_VERSION} for macOS".

Drag the LPub3D Application icon to the Applications folder.

After installation, remove the mounted LPub3D disk image by dragging it to the Trash

Cheers,
EOF
echo "- generate makedmg script..."
cat <<EOF >makedmg
#!/bin/bash
../utilities/create-dmg \\
--volname "LPub3D Installer" \\
--volicon "setup.icns" \\
--dmgicon "setup.png" \\
--background "lpub3dbkg.png" \\
--icon-size 90 \\
--text-size 10 \\
--window-pos 200 120 \\
--window-size 640 480 \\
--icon LPub3D.app 192 344 \\
--hide-extension LPub3D.app \\
--custom-icon Readme README 512 128 \\
--app-drop-link 448 344 \\
--eula .COPYING \\
"${DMGDIR}/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg" \\
DMGSRC/
EOF

echo "- create dmg packages in $(realpath $DMGDIR/)..."
if [ -d DMGSRC/LPub3D.app ]; then
   chmod +x makedmg && ./makedmg
else
  echo "- Could not find LPub3D.app at $(realpath DMGSRC/)"
  echo "- $ME Failed."
  ElapsedTime
  exit 1
fi

if [ -f "${DMGDIR}/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg" ]; then
  cp -f "${DMGDIR}/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg" "${DMGDIR}/LPub3D-UpdateMaster_${LP3D_VERSION}-macos.dmg"
  echo "      Download package..: LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg"
  echo "      Update package....: LPub3D-UpdateMaster_${LP3D_VERSION}-macos.dmg"

  echo "- cleanup..."
  rm -f -R DMGSRC
  rm -f lpub3d.icns lpub3dbkg.png setup.png README .COPYING makedmg
else
  echo "- ${DMGDIR}/LPub3D-${LP3D_APP_VERSION_LONG}-macos.dmg was not found."
  echo "- $ME Failed."
fi

# Elapsed execution time
FinishElapsedTime
