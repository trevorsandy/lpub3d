#!/bin/bash
# generate Makefiles 
cd ../../
qmake -qt=qt5
cd mainApp
qmake -qt=qt5
cd ../quazip
qmake -qt=qt5
cd ../ldrawini
qmake -qt=qt5
