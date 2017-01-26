#!/bin/bash
# Trevor SANDY
# Last Update 27 January 2017
# To run: ./CreateDmg.sh
LOG=~/Projects/lpub3d/builds/osx/CreateDmg.log

echo “Start” > $LOG
echo “Automate the linking of LPub3D additional libraries ldrawini and quazip and production of the LPub3D dmg” >> $LOG

echo “1. Set bundle working directory - build/release” >> $LOG
cd ~/Projects/build-LPub3D-Desktop_Qt_5_7_0_clang_64bit-Release/mainApp/build/release

echo “2. Install library links” >> $LOG
/usr/bin/install_name_tool -id @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/Libs/libldrawini.1.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/Libs/libquazip.1.dylib

echo “3. Change mapping to LPub3D” >> $LOG
/usr/bin/install_name_tool -change libldrawini.1.dylib @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libquazip.1.dylib @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/MacOS/LPub3D

echo “4. Change library dependency mapping” >> $LOG
/usr/bin/install_name_tool -change libldrawini.1.dylib @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libquazip.1.dylib @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo “5. Bundle LPub3D” >> $LOG
~/Qt/IDE/5.7/clang_64/bin/macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

if [ “$1” = “” ]
then
 cd ~/Projects/LPub3D
 PROJECT_VERSION=`git describe --tags --long`
 CLEAN_VERSION=${PROJECT_VERSION%%-*}
 APP_VERSION=UpdateMaster_${CLEAN_VERSION#*v}_osx
else
 APP_VERSION=UpdateMaster_”$1”_osx
fi
echo “6. LPub3D version - ${CLEAN_VERSION#*v}” >> $LOG

echo “7. Set dmg package working directory - LPub3D/builds/osx” >> $LOG
cd ~/Projects/lpub3d/builds/osx
if [ -f LPub3D-$APP_VERSION.dmg ]
then
 rm LPub3D-$APP_VERSION.dmg
fi

echo “8. Create LPub3D-$APP_VERSION.dmg” >> $LOG
mkdir release
ls
/usr/local/bin/appdmg lpub3d.json release/LPub3D-$APP_VERSION.dmg

echo “End!” >> $LOG
