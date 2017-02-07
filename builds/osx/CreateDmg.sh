#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

WORK_DIR=`pwd`
BUILD_DATE=`date "+%Y%m%d"`
# logging stuff
LOG="${WORK_DIR}/CreateDmg.log"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

echo "Start..."
export PATH=~/Qt/IDE/5.7/clang_64:~/Qt/IDE/5.7/clang_64/bin:$PATH

echo "1. create DMG build working directory"
if [ ! -d dmgbuild ]
then
    mkdir dmgbuild
fi
cd dmgbuild

echo "2. download source"
if [ -d lpub3d ]
then
    rm -rf lpub3d
fi
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. capture version and date info"
cd lpub3d
if [ "$1" = "" ]
then
     PROJECT_VERSION="builds/utilities/version_info_posix"
     OLDIFS=$IFS
     IFS='-'
     [ ! -f ${PROJECT_VERSION} ] && { echo "${PROJECT_VERSION} file not found"; exit 99; }
     while read S_VERSION REVISION BUILD S_SHA_HASH 
     do 
        VERSION=${S_VERSION//'"'}
        APP_VERSION=${VERSION}"."${BUILD}
        APP_VERSION_LONG=${VERSION}"."${REVISION}"."${BUILD}_${BUILD_DATE}
        echo "REVISION...........${REVISION}"
        echo "BUILD..............${BUILD}"
        echo "SHA_HASH...........${S_SHA_HASH//'"'}"
     done < ${PROJECT_VERSION}
     IFS=$OLDIFS
else
     VERSION=$1
     APP_VERSION=$1
     APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi
echo "BUILD_DATE.........${BUILD_DATE}"
echo "VERSION............${VERSION}"		
echo "APP_VERSION .......${APP_VERSION}"		
echo "APP_VERSION_LONG...${APP_VERSION_LONG}"

echo "4. get ldraw archive libraries"
curl "http://www.ldraw.org/library/updates/complete.zip" -o "mainApp/extras/complete.zip"
curl "http://www.ldraw.org/library/unofficial/ldrawunf.zip" -o "mainApp/extras/lpub3dldrawunf.zip"

echo "5. configure source"
qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 && /usr/bin/make qmake_all

echo "6. build source"
make

echo "7. copy LPub3D bundle components"
cd builds/osx
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

echo "Finished!"
mv $LOG "${WORK_DIR}/dmgbuild/CreateDmg.log"
