#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

WORK_DIR=`pwd`
LOG="${WORK_DIR}/CreateDmg.log"
BUILD_DATE=`date "+%Y%m%d"`

echo "Start" > $LOG
export PATH=~/Qt/IDE/5.7/clang_64:~/Qt/IDE/5.7/clang_64/bin:$PATH

echo "1. create DMG build working directory" >> $LOG
if [ ! -d dmgbuild ]
then
    mkdir dmgbuild
fi
cd dmgbuild

echo "2. download source" >> $LOG
git clone https://github.com/trevorsandy/lpub3d.git

echo "3. capture version and date info" >> $LOG
cd lpub3d
if [ "$1" = "" ]
then
     PROJECT_VERSION="builds/utilities/version_info_posix"
     OLDIFS=$IFS
     IFS='-'
     [ ! -f ${PROJECT_VERSION} ] && { echo "${PROJECT_VERSION} file not found"; exit 99; }
     while read S_VERSION S_REVISION S_BUILD S_SHA_HASH 
     do 
        VERSION=${S_VERSION//'"'}
        APP_VERSION=${S_VERSION//'"'}"."${S_BUILD}
        APP_VERSION_LONG=${S_VERSION//'"'}"."${S_REVISION}"."${S_BUILD}_${BUILD_DATE}
        echo "BUILD_DATE.........${BUILD_DATE}"
        echo "APP_VERSION_LONG...${APP_VERSION_LONG}"
        echo "APP_VERSION .......${APP_VERSION}"
        echo "VERSION............${S_VERSION//'"'}"
        echo "REVISION...........${S_REVISION}"
        echo "BUILD..............${S_BUILD}"
        echo "SHA_HASH...........${S_SHA_HASH}"
     done < ${PROJECT_VERSION}
     IFS=$OLDIFS
else
     VERSION=$1
     APP_VERSION=$1
     APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi

echo "4. get ldraw archive libraries" >> $LOG
curl "http://www.ldraw.org/library/updates/complete.zip" -o "mainApp/extras/complete.zip"
curl "http://www.ldraw.org/library/unofficial/ldrawunf.zip" -o "mainApp/extras/lpub3dldrawunf.zip"

echo "5. configure source" >> $LOG
qmake LPub3D.pro -spec macx-clang CONFIG+=x86_64 && /usr/bin/make qmake_all

echo "6. build source" >> $LOG
make

echo "7. copy LPub3D bundle components" >> $LOG
cd builds/osx
if [ -d LPub3D.app ]
then
    rm -R LPub3D.app
fi
cp ../../mainApp/lpub3d.icns .
cp ../utilities/icons/lpub3dbkg.png .
cp -R ../../mainApp/release/LPub3D.app .

echo "8. install library links" >> $LOG
/usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni161.1.dylib LPub3D.app/Contents/Libs/libLDrawIni161.1.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP07.1.dylib LPub3D.app/Contents/Libs/libQuaZIP07.1.dylib

echo "9. change mapping to LPub3D" >> $LOG
/usr/bin/install_name_tool -change libLDrawIni161.1.dylib @executable_path/../Libs/libLDrawIni161.1.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libQuaZIP07.1.dylib @executable_path/../Libs/libQuaZIP07.1.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "10. bundle LPub3D" >> $LOG
macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "11. change library dependency mapping" >> $LOG
/usr/bin/install_name_tool -change libLDrawIni161.1.dylib @executable_path/../Libs/libLDrawIni161.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libQuaZIP07.1.dylib @executable_path/../Libs/libQuaZIP07.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "12. generate lpub3d.json and README.txt" >> $LOG
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

echo "13. create dmg packages" >> $LOG
DMGDIR=../../../DMGS
if [ -d ${DMGDIR} ] 
then
    rm -R ${DMGDIR}
fi

mkdir ${DMGDIR}
echo "    Download file: LPub3D_${APP_VERSION_LONG}_osx.dmg" >> $LOG
/usr/local/bin/appdmg lpub3d.json "${DMGDIR}/LPub3D_${APP_VERSION_LONG}_osx.dmg"

echo "      Update file: LPub3D-UpdateMaster_${VERSION}_osx.dmg" >> $LOG
cp "${DMGDIR}/LPub3D-UpdateMaster_${VERSION}_osx.dmg" "${DMGDIR}/LPub3D-UpdateMaster_${VERSION}_osx.dmg"

echo "14. cleanup" >> $LOG
rm -R LPub3D.app
rm lpub3d.icns README.txt lpub3d.json lpub3dbkg.png

echo "Finished!" >> $LOG
mv $LOG "${WORK_DIR}/dmgbuild/CreateDmg.log"
