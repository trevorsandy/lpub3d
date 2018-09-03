#!/bin/bash
# Trevor SANDY
# Last Update June 26, 2018
# LPub3D Unix checks
# NOTE: this file must be sourced. with ${SOURCE_DIR} and ${PUB3D_EXE} predefined.

LP3D_CHECK_FILE="${SOURCE_DIR}/builds/check/build_checks.mpd"

echo "- file process check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-file --preferred-renderer native"
${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE

echo && echo "- export check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-file --clear-cache --preferred-renderer ldview"
${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE

echo && echo "- range check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-export --range 1-3 --clear-cache --preferred-renderer ldglite"
${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE

echo && echo "- povray check..."
LP3D_CHECK_OPTIONS="--ignore-console-redirect --process-file --clear-cache --preferred-renderer povray"
${LPUB3D_EXE} $LP3D_CHECK_OPTIONS $LP3D_CHECK_FILE
