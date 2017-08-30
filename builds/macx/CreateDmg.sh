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
echo "Enter n to download source or any key "
echo "to use existing source if available."
read -n 1 -p "Do you want to continue with these options? : " input
echo

echo "1. create DMG build working directory"
if [ ! -d dmgbuild ]
then
    mkdir dmgbuild
fi
cd dmgbuild

if [ ! -d lpub3d_macos_3rdparty ]
then
    echo "2a. download lpub3d 3rd party renderers"
    git clone https://github.com/trevorsandy/lpub3d_macos_3rdparty.git
  else
    echo "2a. renderer folder exist. skipping download"
fi

if [ -d lpub3d ] && [ "$input" = "n" ] || [ "$input" = "N" ]
then
    echo "2b. download lpub3d source"
    rm -rf lpub3d
    git clone https://github.com/trevorsandy/lpub3d.git
  else
    echo "2b. lpubsource exist skipping download"
fi

echo "2c. checkout integrate-renderers"
cd lpub3d
## TEMPERARY ##
#git checkout integrate-renderers

echo "3. capture version and date info"
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
echo "VER_MAJOR.........${VER_MAJOR}"
echo "VER_MINOR.........${VER_MINOR}"
echo "VER_PATCH.........${VER_PATCH}"
echo "VER_REVISION......${VER_REVISION}"
echo "VER_BUILD.........${VER_BUILD}"
echo "VER_SHA_HASH......${VER_SHA_HASH}"
echo "VERSION...........${VERSION}"
echo "APP_VERSION.......${APP_VERSION}"
echo "APP_VERSION_LONG..${APP_VERSION_LONG}"
echo "BUILD_DATE........${BUILD_DATE}"

if [ ! -f "mainApp/extras/complete.zip" ]
then
  echo "4a. get ldraw official library archive"
  curl "http://www.ldraw.org/library/updates/complete.zip" -o "mainApp/extras/complete.zip"
fi
if [ ! -f "mainApp/extras/lpub3dldrawunf.zip" ]
then
  echo "4b. get ldraw unofficial library archive"
  curl "http://www.ldraw.org/library/unofficial/ldrawunf.zip" -o "mainApp/extras/lpub3dldrawunf.zip"
fi

echo "5. configure source"
qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 && /usr/bin/make qmake_all

echo "6. build source"
make

echo "7. copy LPub3D bundle components"
cd builds/macx
if [ -d LPub3D.app ]
then
    rm -R LPub3D.app
fi
cp ../../mainApp/lpub3d.icns .
cp ../utilities/icons/lpub3dbkg.png .
cp -R ../../mainApp/release/LPub3D.app .

echo "8. install library links"
/usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Libs/libQuaZIP.0.dylib

echo "9. change mapping to LPub3D"
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "10. bundle LPub3D"
macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "11. change library dependency mapping"
/usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "12. generate lpub3d.json and README.txt"
cat <<EOF >lpub3d.json
{
  "title": "LPub3D ${APP_VERSION}",
  "icon": "lpub3d.icns",
  "background": "lpub3dbkg.png",
  "contents": [
    { "x": 448, "y": 344, "type": "link", "path": "/Applications" },
    { "x": 192, "y": 344, "type": "file", "path": "LPub3D.app" },
    { "x": 512, "y": 128, "type": "file", "path": "README.txt" },
    { "x": 512, "y": 900, "type": "position", "path": ".VolumeIcon.icns" }
  ]
}
EOF

cat <<EOF >README.txt
Thank you for installing LPub3D v${APP_VERSION} for OSX".

Drag the LPub3D Application icon to the Applications folder.

After installation, remove the mounted LPub3D disk image by dragging it to the Trash

Cheers,
EOF

echo "13. create dmg packages"
DMGDIR=../../../DMGS
if [ -d ${DMGDIR} ]
then
    rm -R ${DMGDIR}
fi

mkdir ${DMGDIR}
echo "    Download package: LPub3D_${APP_VERSION_LONG}_osx.dmg"
/usr/local/bin/appdmg lpub3d.json "${DMGDIR}/LPub3D_${APP_VERSION_LONG}_osx.dmg"

echo "      Update package: LPub3D-UpdateMaster_${VERSION}_osx.dmg"
cp "${DMGDIR}/LPub3D_${APP_VERSION_LONG}_osx.dmg" "${DMGDIR}/LPub3D-UpdateMaster_${VERSION}_osx.dmg"

echo "14. cleanup"
rm -R LPub3D.app
rm lpub3d.icns README.txt lpub3d.json lpub3dbkg.png

echo "$ME Finished!"
# mv $LOG "${CWD}/dmgbuild/$ME.log"
