#!/bin/bash
# Rebuild Qt and Qt Creator from sources on Linux.
#
# Prior required downloads:
# 
#echo "downloading..."
cd $HOME/Downloads
#wget http://download.qt-project.org/archive/qt/5.7/5.7.1/single/qt-everywhere-opensource-src-5.7.1.tar.gz

QT_SRC=qt-everywhere-opensource-src-5.7.1
QT_INSTALL_ROOT=$HOME/Qt/5.7/gcc_64
QT_INSTALL_PREFIX=$HOME/Qt/5.7/static/gcc_64


echo "Create Qt installation directories:"
install -d -o $USER -m 755 $QT_INSTALL_PREFIX 

# This script:
SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
SCRIPT_NAME=$(basename ${BASH_SOURCE[0]} .sh)

# Exit script on error, redirect output and error in a log file
set -e
exec </dev/null &>$SCRIPT_DIR/$SCRIPT_NAME.log

echo "Extract Qt sources..."
cd $SCRIPT_DIR
tar xf $QT_SRC.tar.gz
cd $QT_SRC

echo "Build and install Qt..."
./configure -prefix $QT_INSTALL_PREFIX -opensource -confirm-license -release -static -platform linux-g++ -accessibility -qt-zlib -qt-libpng -qt-libjpeg -qt-xcb -qt-pcre -qt-freetype -opengl -make libs -force-pkg-config -no-egl -no-cups -no-sql-sqlite -no-glib -no-eglfs -no-qml-debug -no-xinput2 -no-sm -nomake tools -nomake examples -nomake tests -skip qtwebchannel -skip qtwebengine -skip qtwayland
# Old Reference
#./configure -prefix $QT_INSTALL_ROOT -opensource -confirm-license -nomake examples -nomake tests
make -j6
make install

echo "finshed."
