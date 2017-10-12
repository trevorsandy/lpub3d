#!/bin/bash
# Trevor SANDY
# Last Update 28 August 2017
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
BUILD_DATE=`date "+%Y%m%d"`
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

echo "Start $ME..."
export PATH=~/Qt/IDE/5.7/clang_64:~/Qt/IDE/5.7/clang_64/bin:$PATH

echo
echo "Enter n to download LPub3D source or any key to"
echo "skip download and use existing source if available."
read -n 1 -p "Do you want to continue with this option? : " input
echo

echo "01.  create DMG build working directory..."
if [ ! -d dmgbuild ]
then
    mkdir dmgbuild
fi
cd dmgbuild

if [ ! -d lpub3d_macos_3rdparty ]
then
    echo "02.  download LPub3D 3rd party renderers..."
    git clone https://github.com/trevorsandy/lpub3d_macos_3rdparty.git
else
    echo "02.  3rd party renderer folder exist. skipping download..."
fi

if [ -d lpub3d ] && [ "$input" = "n" ] || [ "$input" = "N" ]
then
    echo "03.  download LPub3D source..."
    rm -rf lpub3d
    git clone https://github.com/trevorsandy/lpub3d.git
elif [ ! -d lpub3d ]
then
    echo "03.  download LPub3D source..."
    git clone https://github.com/trevorsandy/lpub3d.git
else
    echo "03.  LPub3D source exist. skipping download"
fi

echo "04.  checkout integrate-renderers..."
cd lpub3d
## TEMPERARY ##
#git checkout integrate-renderers

echo "05.  capture version and date info..."
#         1 2  3  4   5       6    7  8  9       10
# format "2 0 20 17 663 410fdd7 2017 02 12 19:50:21"
FILE="builds/utilities/version.info"
if [ -f ${FILE} -a -r ${FILE} ]
then
    VERSION_INFO=`cat ${FILE}`
else
    echo "Error: Cannot read ${FILE} from `pwd`"
    echo "$ME terminated!"
    exit 1
fi
read VER_MAJOR VER_MINOR VER_PATCH VER_REVISION  VER_BUILD VER_SHA_HASH <<< ${VERSION_INFO//'"'}
VERSION=${VER_MAJOR}"."${VER_MINOR}"."${VER_PATCH}
APP_VERSION=${VERSION}"."${VER_BUILD}
APP_VERSION_LONG=${VERSION}"."${VER_REVISION}"."${VER_BUILD}_${BUILD_DATE}
#echo "WORK_DIR..........${WORK_DIR}"
echo "    VER_MAJOR.........${VER_MAJOR}"
echo "    VER_MINOR.........${VER_MINOR}"
echo "    VER_PATCH.........${VER_PATCH}"
echo "    VER_REVISION......${VER_REVISION}"
echo "    VER_BUILD.........${VER_BUILD}"
echo "    VER_SHA_HASH......${VER_SHA_HASH}"
echo "    VERSION...........${VERSION}"
echo "    APP_VERSION.......${APP_VERSION}"
echo "    APP_VERSION_LONG..${APP_VERSION_LONG}"
echo "    BUILD_DATE........${BUILD_DATE}"

if [ ! -f "mainApp/extras/complete.zip" ]
then
  echo "06.  get ldraw official library archive..."
  curl "http://www.ldraw.org/library/updates/complete.zip" -o "mainApp/extras/complete.zip"
else
  echo "06.  ldraw official library exist. skipping download"
fi
if [ ! -f "mainApp/extras/lpub3dldrawunf.zip" ]
then
  echo "07.  get ldraw unofficial library archive..."
  curl "http://www.ldraw.org/library/unofficial/ldrawunf.zip" -o "mainApp/extras/lpub3dldrawunf.zip"
else
  echo "07.  ldraw unofficial library exist. skipping download"
fi

echo "08.  configure and build LPub3D source..."
qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 && /usr/bin/make qmake_all

# create dmg environment - begin #
#
cd builds/macx

echo "09. copy LPub3D bundle components..."
cp -f -R ../../mainApp/release/LPub3D.app .
cp -f ../utilities/icons/lpub3d.icns .
cp -f ../utilities/icons/lpub3dbkg.png .
cp -f ../utilities/COPYING_BRIEF .COPYING

echo "10. install library links..."
/usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Libs/libQuaZIP.0.dylib

echo "11. change mapping to LPub3D..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "12. bundle LPub3D..."
macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "13. change library dependency mapping..."
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "14. setup dmg source dir..."
if [ ! -d DMGSRC ]
then
    mkdir DMGSRC
fi
mv -f LPub3D.app DMGSRC/LPub3D.app

echo "15. setup dmg output dir..."
DMGDIR=../../../DMGS
if [ -d ${DMGDIR} ]
then
    rm -f -R ${DMGDIR}
fi
mkdir -p ${DMGDIR}

# pos: builds/macx
echo "16. generate README file and dmg make script..."
cat <<EOF >README
Thank you for installing LPub3D v${APP_VERSION} for MacOS".

Drag the LPub3D Application icon to the Applications folder.

After installation, remove the mounted LPub3D disk image by dragging it to the Trash

Cheers,
EOF

cat <<EOF >makedmg
#!/bin/bash
../utilities/create-dmg \\
--volname "LPub3D Installer" \\
--volicon "lpub3d.icns" \\
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
"${DMGDIR}/LPub3D_${APP_VERSION_LONG}_osx.dmg" \\
DMGSRC/
EOF

echo "17. create dmg packages..."
chmod +x makedmg && ./makedmg

cp "${DMGDIR}/LPub3D_${APP_VERSION_LONG}_osx.dmg" "${DMGDIR}/LPub3D-UpdateMaster_${VERSION}_osx.dmg"
echo "      Download package..: LPub3D_${APP_VERSION_LONG}_osx.dmg"
echo "      Update package....: LPub3D-UpdateMaster_${VERSION}_osx.dmg"

echo "18. cleanup..."
rm -f -R DMGSRC
rm -f lpub3d.icns lpub3dbkg.png README .COPYING makedmg

# create dmg environment - end #

echo "$ME Finished!"
# mv $LOG "${CWD}/dmgbuild/$ME.log"
