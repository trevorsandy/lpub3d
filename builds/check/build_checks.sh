#!/bin/bash
# Trevor SANDY
# Last Update July 23, 2019
# Copyright (c) 2018 - 2020 by Trevor SANDY
# LPub3D Unix checks - for remote CI (Trevis, OBS)
# NOTE: Source with variables as appropriate:
#       $BUILD_OPT = compile
#       $XMING = true,
#       $LP3D_BUILD_APPIMAGE = true,
#       $SOURCE_DIR = <lpub3d source folder>

# Initialize build check elapsed time
lp3d_elapsed_check_start=$SECONDS

# Build Check Timer args: 1 = <start> (seconds mark)
ElapsedCheckTime() {
  if test -z "$1"; then return 0; fi
  TIME_ELAPSED="$(((SECONDS - $1) % 60))sec"
  TIME_MINUTES=$((((SECONDS - $1) / 60) % 60))
  TIME_HOURS=$(((SECONDS - $1) / 3600))
  if [ "$TIME_MINUTES" -gt 0 ]; then
    TIME_ELAPSED="${TIME_MINUTES}mins $TIME_ELAPSED"
  fi
  if [ "$TIME_HOURS" -gt 0 ]; then
    TIME_ELAPSED="${TIME_HOURS}hrs $TIME_ELAPSED"
  fi
  echo "$TIME_ELAPSED"
}

# Initialize platform variables
LP3D_OS_NAME=$(uname)
LP3D_TARGET_ARCH="$(uname -m)"
LP3D_PLATFORM=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $OS_NAME | awk '{print tolower($0)}')

# Set XDG_RUNTIME_DIR
if [[ "${LP3D_OS_NAME}" != "Darwin" && $UID -ge 1000 && -z "$(printenv | grep XDG_RUNTIME_DIR)" ]]; then
    runtime_dir="/tmp/runtime-user-$UID"
    if [ ! -d "$runtime_dir" ]; then
       mkdir -p $runtime_dir
    fi
    export XDG_RUNTIME_DIR="$runtime_dir"
fi

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

# Initialize variables
LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/build_checks.mpd"
LP3D_CHECK_STDLOG="$(realpath ${SOURCE_DIR})/builds/check/LPub3D"
LP3D_CHECK_SUCCESS="Application terminated with return code 0."
LP3D_LOG_FILE="Check.out"
let LP3D_CHECK_PASS=0
let LP3D_CHECK_FAIL=0
LP3D_CHECKS_PASS=
LP3D_CHECKS_FAIL=

echo && echo "------------Build Checks Start--------------" && echo

# Remove old log if exist
if [ -f "${LP3D_LOG_FILE}" ]; then
    rm -rf "${LP3D_LOG_FILE}"
fi

for LP3D_BUILD_CHECK in CHECK01 CHECK02 CHECK03 CHECK04 CHECK05 CHECK06 CHECK07; do
    lp3d_check_start=$SECONDS
    case ${LP3D_BUILD_CHECK} in
    CHECK01)
        LP3D_CHECK_LBL="Native File Process"
        LP3D_CHECK_HDR="- Check 1 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --liblego --preferred-renderer native"
        LP3D_CHECK_STDLOG=
        ;;
    CHECK02)
        LP3D_CHECK_LBL="LDView File Process"
        LP3D_CHECK_HDR="- Check 2 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer ldview"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        ;;
    CHECK03)
        LP3D_CHECK_LBL="LDView (Single Call) File Process"
        LP3D_CHECK_HDR="- Check 3 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer ldview-sc"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        ;;
    CHECK04)
        LP3D_CHECK_LBL="LDGLite Export Range"
        LP3D_CHECK_HDR="- Check 4 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stderr-ldglite"
        ;;
    CHECK05)
        LP3D_CHECK_LBL="Native POV Generation"
        LP3D_CHECK_HDR="- Check 5 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer povray"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stderr-povray"
        ;;
    CHECK06)
        LP3D_CHECK_LBL="LDView TENTE Model"
        LP3D_CHECK_HDR="- Check 6 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --libtente --preferred-renderer ldview"
        LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/TENTE/astromovil.ldr"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        ;;
    CHECK07)
        LP3D_CHECK_LBL="LDView (Snapshot List) VEXIQ Model"
        LP3D_CHECK_HDR="- Check 7 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --libvexiq --preferred-renderer ldview-scsl"
        LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/VEXIQ/spider.mpd"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        ;;
      esac

    [ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} &> ${LP3D_LOG_FILE} || \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} &> ${LP3D_LOG_FILE}
    # allow some time between checks
    sleep 4
    # check output log for build check status
    if [ -f "${LP3D_LOG_FILE}" ]; then
        if grep -q "${LP3D_CHECK_SUCCESS}" "${LP3D_LOG_FILE}"; then
            echo "${LP3D_CHECK_HDR} PASSED, ELAPSED TIME [`ElapsedCheckTime $lp3d_check_start`]"
            echo "${LP3D_CHECK_LBL} Command: ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}"
            let LP3D_CHECK_PASS++
            LP3D_CHECKS_PASS="${LP3D_CHECKS_PASS},$(echo ${LP3D_CHECK_HDR} | cut -d " " -f 3)"
            echo
        else
            echo "${LP3D_CHECK_HDR} FAILED, ELAPSED TIME [`ElapsedCheckTime $lp3d_check_start`]"
            let LP3D_CHECK_FAIL++
            LP3D_CHECKS_FAIL="${LP3D_CHECKS_FAIL},$(echo ${LP3D_CHECK_HDR} | cut -d " " -f 3)"
            echo "- LPub3D Log Trace: ${LP3D_LOG_FILE}"
            cat "${LP3D_LOG_FILE}"
            [ -f "${LP3D_CHECK_STDLOG}" ] && \
            echo "- Standard Error Log Trace: ${LP3D_CHECK_STDLOG}" && \
            cat "${LP3D_CHECK_STDLOG}" || true
            echo
        fi
        rm -rf "${LP3D_LOG_FILE}"
    else
        echo "ERROR - ${LP3D_LOG_FILE} not found."
    fi
done

# remove leading ','
if [ "${LP3D_CHECK_PASS}" -gt "0" ];then
    LP3D_CHECKS_PASS="$(echo ${LP3D_CHECKS_PASS} | cut -c 2-)"
fi
if [ "${LP3D_CHECK_FAIL}" -gt "0" ];then
    LP3D_CHECKS_FAIL="$(echo ${LP3D_CHECKS_FAIL} | cut -c 2-)"
fi

SUMMARY_MSG=''
SUMMARY_MSG+="----Build Check Completed: PASS (${LP3D_CHECK_PASS})[${LP3D_CHECKS_PASS}], "
SUMMARY_MSG+="FAIL (${LP3D_CHECK_FAIL})[${LP3D_CHECKS_FAIL}], "
SUMMARY_MSG+="ELAPSED TIME [`ElapsedCheckTime $lp3d_elapsed_check_start`]----"
echo && echo $SUMMARY_MSG && echo

