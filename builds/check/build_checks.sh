#!/bin/bash
# Trevor SANDY
# Last Update October 21, 2018
# LPub3D Unix checks - for remote CI (Trevis, OBS) 
# NOTE: Source with variables as appropriate:
#       $BUILD_OPT = compile
#       $XMING = true,
#       $LP3D_BUILD_APPIMAGE = true,
#       $SOURCE_DIR = <lpub3d source folder>

# Initialize platform variables
LP3D_OS_NAME=$(uname)
LP3D_TARGET_ARCH="$(uname -m)"

# Initialize XVFB 
if [[ "${XMING}" != "true" && ("${DOCKER}" = "true" || ("${LP3D_OS_NAME}" != "Darwin")) ]]; then
    echo && echo "- Using XVFB from working directory: ${PWD}"
    USE_XVFB="true"
fi

# macOS, compile only, initialize build paths and libraries
if [[ "${LP3D_OS_NAME}" = "Darwin" && "$BUILD_OPT" = "compile" ]]; then
    cd ${SOURCE_DIR}/mainApp/64bit_release

    echo "- set macOS LPub3D executable..."
    LPUB3D_EXE="LPub3D.app/Contents/MacOS/LPub3D"

    echo "- install library links..."
    /usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
    /usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Libs/libQuaZIP.0.dylib

    echo "- change mapping to LPub3D..."
    /usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
    /usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/MacOS/LPub3D

    echo "- bundle LPub3D..."
    macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

    echo "- change library dependency mapping..."
    /usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
    /usr/bin/install_name_tool -change libQuaZIP.0.dylib @executable_path/../Libs/libQuaZIP.0.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
fi

# AppImage execute permissions
if [ "$LP3D_BUILD_APPIMAGE" = "true" ]; then
    cd ${SOURCE_DIR}
     
    echo && echo "- set AppImage $(realpath ${LPUB3D_EXE}) execute permissions..."
    [ -f "${LPUB3D_EXE}" ] && \
    chmod u+x ${LPUB3D_EXE} || \
    echo "ERROR - $(realpath ${LPUB3D_EXE}) not found." 
fi

echo && echo "------------Build checks start--------------" && echo

LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/build_checks.mpd"

for LP3D_BUILD_CHECK in CHECK01 CHECK02 CHECK03 CHECK04 CHECK05 CHECK06 CHECK07; do
    case ${LP3D_BUILD_CHECK} in
    CHECK01)
        LP3D_CHECK_HDR="- Check 1 of 7: Native File Process Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --liblego --preferred-renderer native"
        ;;
    CHECK02)
        LP3D_CHECK_HDR="- Check 2 of 7: LDView File Process Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer ldview"
        ;;
    CHECK03)
        LP3D_CHECK_HDR="- Check 3 of 7: LDView (Single Call) File Process Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer ldview-sc"
        ;;
    CHECK04)
        LP3D_CHECK_HDR="- Check 4 of 7: LDGLite Export Range Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite"
        ;;
    CHECK05)
        LP3D_CHECK_HDR="- Check 5 of 7: Native POV Generation Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer povray"
        ;;
    CHECK06)
        LP3D_CHECK_HDR="- Check 6 of 7: LDView TENTE Model Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --libtente --preferred-renderer ldview"
        LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/TENTE/astromovil.ldr"
        ;;
    CHECK07)
        LP3D_CHECK_HDR="- Check 7 of 7: Native VEXIQ Model Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --libvexiq --preferred-renderer native"
        LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/VEXIQ/spider.mpd"
        ;;
      esac

    echo && echo ${LP3D_CHECK_HDR}
    #echo "  Executable: $(realpath ${LPUB3D_VER})"
    [ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} || \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}
done

echo && echo "------------Build checks completed----------" && echo

