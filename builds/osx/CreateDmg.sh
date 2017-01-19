#!/bin/bash
# To run: ./CreateDmg.sh [<version>]
LOG_DIR=/Users/trevorsandy/Projects/LPub3D_X11/builds

echo “Start” > $LOG_DIR/CreateDmg.log
echo “Automate the linking of LPub3D additional libraries ldrawini and quazip and production of the LPub3D dmg” >> $LOG_DIR/CreateDmg.log

echo “1. Set bundle working directory - build/release” >> $LOG_DIR/CreateDmg.log
cd /Users/trevorsandy/Projects/build-LPub3D-Desktop_Qt_5_7_0_clang_64bit-Release/mainApp/build/release

echo “2. Install library links” >> $LOG_DIR/CreateDmg.log
/usr/bin/install_name_tool -id @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/Libs/libldrawini.1.dylib
/usr/bin/install_name_tool -id @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/Libs/libquazip.1.dylib

echo “3. Change mapping to LPub3D” >> $LOG_DIR/CreateDmg.log
/usr/bin/install_name_tool -change libldrawini.1.dylib @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/MacOS/LPub3D
/usr/bin/install_name_tool -change libquazip.1.dylib @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/MacOS/LPub3D

echo “4. Change library dependency mapping” >> $LOG_DIR/CreateDmg.log
/usr/bin/install_name_tool -change libldrawini.1.dylib @executable_path/../Libs/libldrawini.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
/usr/bin/install_name_tool -change libquazip.1.dylib @executable_path/../Libs/libquazip.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

echo “5. Bundle LPub3D” >> $LOG_DIR/CreateDmg.log
/Users/trevorsandy/Qt/IDE/5.7/clang_64/bin/macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

echo “6. Package LPub3D - create dmg file” >> $LOG_DIR/CreateDmg.log
if [ “$1” = “” ]
then
 APP_VERSION=2.0.X
else
 APP_VERSION="$1"
fi

echo “6a. Set dmg file working directory - LPub3D_X11/builds” >> $LOG_DIR/CreateDmg.log
cd /Users/trevorsandy/Projects/LPub3D_X11/builds
if [ -f LPub3D_$APP_VERSION.dmg ]
then
 echo “6b. Remove old dmg file” >> $LOG_DIR/CreateDmg.log
 rm LPub3D_$APP_VERSION.dmg
fi

echo “6c. Create dmg file” >> $LOG_DIR/CreateDmg.log
/usr/local/bin/appdmg lpub3d.json /Users/trevorsandy/Downloads/LPub3D_$APP_VERSION.dmg

echo “End!” >> $LOG_DIR/CreateDmg.log
