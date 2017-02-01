#!/bin/bash
# Trevor SANDY
# Last Update 30 January 2017
# To run:
# $ chmod 755 CreateDmg.sh
# $ ./CreateDmg.sh

WORK_DIR=`pwd`
LOG="${WORK_DIR}/CreateDmg.log"
BUILD_DATE=`date "+%Y%m%d"`

if [ "$1" = "" ]
then
 PROJECT_VERSION=`cat ../utilities/version_info_posix`
 IFS=- read VERSION REVISION BUILD SHA_HASH <<< ${PROJECT_VERSION//'"'}
 APP_VERSION=${VERSION}"."${BUILD}
 APP_VERSION_LONG=${VERSION}"."${REVISION}"."${BUILD}_${BUILD_DATE}
else
 VERSION=$1
 APP_VERSION=$1
 APP_VERSION_LONG=$1"_"${BUILD_DATE}
fi

echo "Start" > $LOG
echo "Automate the linking of LPub3D additional libraries ldrawini and quazip and production of the LPub3D dmg" >> $LOG

echo "1. Copy LPub3D bundle to working directory" >> $LOG
if [ -d LPub3D.app ]
then
    rm -R LPub3D.app
fi
cp ~/Projects/LPub3D/mainApp/lpub3d.icns .
cp ~/Projects/LPub3D/builds/utilities/icons/lpub3dbkg.png .
cp -R ~/Projects/build-LPub3D-Desktop_Qt_5_7_0_clang_64bit-Release/mainApp/build/release/LPub3D.app .

echo "2. Install library links" >> $LOG
/usr/bin/install_name_tool -id @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/Libs/libldrawini.1.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/Libs/libquazip.1.dylib

echo "3. Change mapping to LPub3D" >> $LOG
/usr/bin/install_name_tool -change libldrawini.1.dylib @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libquazip.1.dylib @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/MacOS/LPub3D

echo "4. Bundle LPub3D" >> $LOG
~/Qt/IDE/5.7/clang_64/bin/macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo "5. Change library dependency mapping" >> $LOG
/usr/bin/install_name_tool -change libldrawini.1.dylib @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libquazip.1.dylib @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo "6. Generate lpub3d.json and README.txt" >> $LOG
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

echo "7. Cleanup" >> $LOG
rm -R LPub3D.app
rm lpub3d.icns README.txt lpub3d.json lpub3dbkg.png

echo "  Update file: LPub3D-UpdateMaster_${VERSION}_osx.dmg" >> $LOG
if [ -d release ]
then
    rm -R release
    mkdir release
else
    mkdir release
fi
/usr/local/bin/appdmg lpub3d.json release/LPub3D-UpdateMaster_${VERSION}_osx.dmg

echo "Download file: LPub3D_${APP_VERSION_LONG}_osx.dmg" >> $LOG
cp -R release/LPub3D-UpdateMaster_${VERSION}_osx.dmg release/LPub3D_${APP_VERSION_LONG}_osx.dmg

echo "Finished!" >> $LOG
mv $LOG "${WORK_DIR}/CreateDeb.log"
