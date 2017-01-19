#!/bin/bash
# Version Standard User
# This script requires QtInstallerFramework 2.0 or greater

echo “Start”

if [ “$1” = “” ]
then
 APP_VERSION=2.0.XX
else
 APP_VERSION="$1"
fi
BUILD_DATE=`date +%Y-%m-%d`

# Paths
BUILD_PATH=/home/trevor/projects/lpub3d_x11/builds/linux
APP_PACKAGE=/packages/com.lpub3d.root.application
DATA_PACKAGE=/packages/com.lpub3d.root.application.data
DESK_PACKAGE=/packages/com.lpub3d.root.application.desktop
ICON_S_PACKAGE=/packages/com.lpub3d.root.application.iconsuse
ICON_U_PACKAGE=/packages/com.lpub3d.root.application.iconubuntu
COMPILE_PATH=/home/trevor/projects/build-LPub3Dx11-Desktop_Qt_5_7_1_GCC_64bit-Release
QT_PLUGIN=/home/trevor/Qt/5.7/gcc_64/plugins
QT_LIB=/home/trevor/Qt/5.7/gcc_64/lib
USR_LIB=/usr/lib/x86_64-linux-gnu
OS_LIB=/lib/x86_64-linux-gnu
DEV_MAIN_APP=/home/trevor/projects/lpub3d_x11/mainApp
DEV_ICONS=/home/trevor/projects/lpub3d_x11/tools/icons
INSTALL_FRAMEWORK=/home/trevor/Qt/Tools/QtInstallerFramework/2.0/bin
BUILD_PATH_LIB=$BUILD_PATH/p1/app/lib
BUILD_PATH_PLUGINS=$BUILD_PATH/p1/app/plugins

cd "$BUILD_PATH"

echo “01 of 17 Crerating Directories...”
# Drectories (application)
mkdir "$BUILD_PATH/p1"
mkdir "$BUILD_PATH/p1/doc"
mkdir "$BUILD_PATH/p1/app"
mkdir "$BUILD_PATH/p1/app/lib"
mkdir "$BUILD_PATH/p1/app/plugins"
#mkdir "$BUILD_PATH/p1/app/plugins/platforms"
#mkdir "$BUILD_PATH/p1/app/plugins/xcbglintegrations"
# Drectories (application data)
mkdir "$BUILD_PATH/p2"
mkdir "$BUILD_PATH/p2/extras"
mkdir "$BUILD_PATH/p2/libraries"
# Drectories (icons - ubuntu)
mkdir "$BUILD_PATH/p3"

echo “02 of 17 Copying qt.conf...”
# Paths configuration script
cp "$DEV_MAIN_APP/qt.conf" "$BUILD_PATH/p1/app/qt.conf"
cp "$DEV_MAIN_APP/lpub3d.sh" "$BUILD_PATH/p1/app/lpub3d.sh"

echo “03 of 17 Copying executable...”
# LPub3D executable and qt.conf
cp "$COMPILE_PATH/mainApp/build/release/lpub3d" "$BUILD_PATH/p1/app/lpub3d"

echo “04 of 17 Copying LPub3D libraries...”
# LPub3D libraries
cp -r "$COMPILE_PATH/ldrawini/build/release/"* "$BUILD_PATH_LIB/"
cp -r "$COMPILE_PATH/quazip/build/release/"* "$BUILD_PATH_LIB/"

echo “05 of 17 Copying Qt framework libraries...”
# Qt libraries
cp "$QT_LIB/libQt5PrintSupport.so.5" "$BUILD_PATH_LIB/libQt5PrintSupport.so.5"
cp "$QT_LIB/libQt5Widgets.so.5" "$BUILD_PATH_LIB/libQt5Widgets.so.5"
cp "$QT_LIB/libQt5Network.so.5" "$BUILD_PATH_LIB/libQt5Network.so.5"
cp "$QT_LIB/libQt5OpenGL.so.5" "$BUILD_PATH_LIB/libQt5OpenGL.so.5"
cp "$QT_LIB/libQt5XcbQpa.so.5" "$BUILD_PATH_LIB/libQt5XcbQpa.so.5"
cp "$QT_LIB/libQt5Core.so.5" "$BUILD_PATH_LIB/libQt5Core.so.5"
cp "$QT_LIB/libQt5DBus.so.5" "$BUILD_PATH_LIB/libQt5DBus.so.5"
cp "$QT_LIB/libQt5Gui.so.5" "$BUILD_PATH_LIB/libQt5Gui.so.5"
cp "$QT_LIB/libicui18n.so.56" "$BUILD_PATH_LIB/libicui18n.so.56"
cp "$QT_LIB/libicudata.so.56" "$BUILD_PATH_LIB/libicudata.so.56"
cp "$QT_LIB/libicuuc.so.56" "$BUILD_PATH_LIB/libicuuc.so.56"

echo “07 of 17 Copying Qt plugin libraries...”
# Qt plugins
cp -r "$QT_PLUGIN/xcbglintegrations" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/platforms" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/imageformats" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/printsupport" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/iconengines" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/bearer" "$BUILD_PATH_PLUGINS/"

cp -r "$QT_PLUGIN/egldeviceintegrations" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/platforminputcontexts" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/sceneparsers" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/generic" "$BUILD_PATH_PLUGINS/"
cp -r "$QT_PLUGIN/qmltooling" "$BUILD_PATH_PLUGINS/"

echo “08 of 17 Copying System libraries...“
# System Libraries
cp "$USR_LIB/libstdc++.so.6" "$BUILD_PATH_LIB/libstdc++.so.6"
cp "$OS_LIB/libgcc_s.so.1" "$BUILD_PATH_LIB/libgcc_s.so.1"
cp "$OS_LIB/libm.so.6" "$BUILD_PATH_LIB/libm.so.6"
cp "$OS_LIB/libc.so.6" "$BUILD_PATH_LIB/libc.so.6"

echo “09 of 17 Copying LPub3D documents...”
###### DOCUMENTS ######
cp -r "$DEV_MAIN_APP/docs/"* "$BUILD_PATH/p1/doc/" 

echo “10 of 17 Copying LPub3D parameter files...”
###### PARAMETER FILES ######
cp -r "$DEV_MAIN_APP/extras/"* "$BUILD_PATH/p2/extras/"

echo “11 of 17 Copying LPub3D archive libraries...”
###### LDRAW ARCHIVE LIBRARIES ######
cp -r "/home/trevor/projects/data/libraries/"* "$BUILD_PATH/p2/libraries/"

###### ICONS ######
echo “12 of 17 Copying icon assets...”
# icons - suse
cp "$DEV_MAIN_APP/images/lpub3d.png" "$BUILD_PATH$ICON_S_PACKAGE/data/lpub3d.png"
# icons - ubuntu
cp -r "$DEV_ICONS/hicolor" "$BUILD_PATH/p3/"

###### DESKTOP ######
#echo “13 of 17 Copying desktop shortcut...”
# desktop shortcut and mime
#cp "$DEV_MAIN_APP/lpub3d.desktop.local" "$BUILD_PATH$DESK_PACKAGE/data/lpub3d.desktop"

###### MIME ######
#echo “13 of 17 Copying mime assets...”
# application mime
# cp "$DEV_MAIN_APP/lpub3d.xml" "$BUILD_PATH$???_PACKAGE/data/lpub3d.xml"

###### MAN ######
#echo “13 of 17 Copying man page assets...”
# man page
# cp "$DEV_MAIN_APP/lpub3d.1" "$BUILD_PATH$???_PACKAGE/data/lpub3d.1"

echo “14 of 17 Building data components...”
# archive p1 data - application and dependencies
$INSTALL_FRAMEWORK/archivegen "$BUILD_PATH$APP_PACKAGE/data/docdata.7z" "$BUILD_PATH/p1/doc"
$INSTALL_FRAMEWORK/archivegen "$BUILD_PATH$APP_PACKAGE/data/appdata.7z" "$BUILD_PATH/p1/app"
$INSTALL_FRAMEWORK/archivegen "$BUILD_PATH$DATA_PACKAGE/data/extrasdata.7z" "$BUILD_PATH/p2/extras"
$INSTALL_FRAMEWORK/archivegen "$BUILD_PATH$DATA_PACKAGE/data/librariesdata.7z" "$BUILD_PATH/p2/libraries"
$INSTALL_FRAMEWORK/archivegen "$BUILD_PATH$ICON_U_PACKAGE/data/iconsdata.7z" "$BUILD_PATH/p3/hicolor"

echo “15 of 17 Directory cleanup...”
# Cleanup
cd "$BUILD_PATH"
rm -rf "$BUILD_PATH/p1"
rm -rf "$BUILD_PATH/p2"
rm -rf "$BUILD_PATH/p3"

echo “16 of 17 Building installer...”
# Create installer binary
$INSTALL_FRAMEWORK/binarycreator -c "config/config.xml" -p packages "/home/trevor/Downloads/LPub3D_X11_$APP_VERSION"
echo “17 of 17 Installer LPub3D_X11_$APP_VERSION was successfully compiled!”

echo “Finished.”











