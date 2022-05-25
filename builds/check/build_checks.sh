#!/bin/bash
# Trevor SANDY
# Last Update June 07, 2021
# Copyright (c) 2018 - 2021 by Trevor SANDY
# LPub3D Unix build checks - for remote CI (Travis, OBS)
# NOTE: Source with variables as appropriate:
#       $BUILD_OPT = compile   (macOS only)
#       $XSERVER = true|false  (used when running local XServer)
#       $LP3D_BUILD_OS = appimage|flatpak|snap,
#       $SOURCE_DIR = <lpub3d source folder>
#       $LPUB3D_EXE = <lpub3d executable path>

# Startup
HOME_DIR=$PWD
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
if [ "${ME}" = "build_checks.sh" ]; then
    # not sourced
    SCRIPT_NAME=$0
    SCRIPT_ARGS=$*
    echo && echo "Start $ME execution at $HOME_DIR..."
else
    # sourced
    echo && echo "Start build_checks.sh execution at $HOME_DIR..."
fi

# Initialize build check elapsed time
lp3d_elapsed_check_start=$SECONDS

function show_settings
{
    echo
    echo "--Buld Check Settings:"
    [ -n "${LP3D_BUILD_OS}" ] && echo "--LP3D_BUILD_OS......$LP3D_BUILD_OS"
    echo "--LPUB3D_EXE.........$LPUB3D_EXE"
    echo "--SOURCE_DIR.........$SOURCE_DIR"
    echo "--XDG_RUNTIME_DIR....$XDG_RUNTIME_DIR"
    [ "${USE_XVFB}" = "true" ] && echo "--USE_XVFB...........YES"
    [ "${XSERVER}" = "true" ] && echo "--XSERVER............YES"
    [ "${DOCKER}" = "true" ] && echo "--DOCKER.............YES"
    echo "--LP3D_OS_NAME.......$LP3D_OS_NAME"
    echo "--LP3D_TARGET_ARCH...$LP3D_TARGET_ARCH"
    echo "--LP3D_PLATFORM......$LP3D_PLATFORM"
    [ -n "${LP3D_CHECK_LDD}" ] && echo "--LP3D_CHECK_LDD.....YES"
    [ -n "${SCRIPT_NAME}" ] && echo "--SCRIPT_NAME.......$SCRIPT_NAME"
    echo
}

function show_settings_and_exit
{
    show_settings
    [ -n "$1" ] && echo "$1" && echo
    # Exit and allow the build process to continue
    exit 0
}

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
if [[ "${LP3D_OS_NAME}" = "Darwin" ]]; then
    LP3D_PLATFORM=$(echo `sw_vers -productName`)
else
    LP3D_PLATFORM=$(. /etc/os-release 2>/dev/null; [ -n "$ID" ] && echo $ID || echo $OS_NAME | awk '{print tolower($0)}')
fi

# Flatpak validate and set executable permissions
if [[ "$LP3D_BUILD_OS" = "flatpak" || "$LP3D_BUILD_OS" = "snap" ]]; then
    LPUB3D_EXE=$(find ${LPUB3D_DEST}/bin -name lpub3d* -type f)
    if [[ -f "$LPUB3D_EXE" ]]; then
        if [[ -d "$SOURCE_DIR/builds/check" ]]; then
            cd ${SOURCE_DIR}
        else
            show_settings_and_exit "ERROR - Invalid source path specified. Build check cannot be executed."
        fi
    else
        show_settings_and_exit "ERROR - LPub3D executable was not found. Build check cannot be executed."
    fi
fi

# AppImage validate and set executable permissions
case "${LPUB3D_EXE}" in
*.AppImage)
    VALID_APPIMAGE=1 ;;
*)
    VALID_APPIMAGE=0 ;;
esac
if [[ "$LP3D_BUILD_OS" = "appimage" && $VALID_APPIMAGE -eq 1 ]]; then
    if [[ -f "${LPUB3D_EXE}" ]]; then
        chmod u+x ${LPUB3D_EXE}
        cd ${SOURCE_DIR}
    else
        show_settings_and_exit "ERROR - $(realpath ${LPUB3D_EXE}) not found. Build check cannot be executed."
    fi
fi

# Set XDG_RUNTIME_DIR
VALID_UID=0
if [[ "$LP3D_BUILD_OS" = "snap" ]]; then
    [ -n $UID ] && VALID_UID=1
else
    [ $UID -ge 1000 ] && VALID_UID=1
fi
if [[ "${LP3D_OS_NAME}" != "Darwin" && $VALID_UID -eq 1 && -z "$(printenv | grep XDG_RUNTIME_DIR)" ]]; then
    runtime_dir="/tmp/runtime-user-$UID"
    if [ ! -d "$runtime_dir" ]; then
       mkdir -p $runtime_dir
    fi
    export XDG_RUNTIME_DIR="$runtime_dir"
fi

# Set USE_XVFB flag
if [[ "${XSERVER}" != "true" && ("${DOCKER}" = "true" || ("${LP3D_OS_NAME}" != "Darwin")) ]]; then
    if [ -z "$(command -v xvfb-run)" ]; then
        show_settings_and_exit "ERROR - XVFB (xvfb-run) could not be found. Build check cannot be executed."
    else
        USE_XVFB="true"
    fi
fi

show_settings

# macOS, compile only, initialize build paths and libraries
if [[ "${LP3D_OS_NAME}" = "Darwin" && "$BUILD_OPT" = "compile" ]]; then
    cd ${SOURCE_DIR}/mainApp/64bit_release

    echo "- set macOS LPub3D executable..."
    LPUB3D_EXE="LPub3D.app/Contents/MacOS/LPub3D"

    echo "- install library links..."
    /usr/bin/install_name_tool -id @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Libs/libLDrawIni.16.dylib
    /usr/bin/install_name_tool -id @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/Libs/libQuaZIP.1.dylib

    echo "- change mapping to LPub3D..."
    /usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/MacOS/LPub3D
    /usr/bin/install_name_tool -change libQuaZIP.1.dylib @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/MacOS/LPub3D

    echo "- bundle LPub3D..."
    macdeployqt LPub3D.app -verbose=1 -executable=LPub3D.app/Contents/MacOS/LPub3D -always-overwrite

    echo "- change library dependency mapping..."
    /usr/bin/install_name_tool -change libLDrawIni.16.dylib @executable_path/../Libs/libLDrawIni.16.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
    /usr/bin/install_name_tool -change libQuaZIP.1.dylib @executable_path/../Libs/libQuaZIP.1.dylib LPub3D.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
fi

# Initialize variables
LP3D_CHECK_STATUS=${LP3D_CHECK_STATUS:---version}
LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/build_checks.mpd"
LP3D_CHECK_STDLOG="$(realpath ${SOURCE_DIR})/builds/check/LPub3D"
LP3D_CHECK_SUCCESS="Application terminated with return code 0."
LP3D_XVFB_ERROR="xvfb-run: error: "
LP3D_LOG_FILE="Check.out"
LP3D_ERROR=0
let LP3D_CHECK_PASS=0
let LP3D_CHECK_FAIL=0
LP3D_CHECKS_PASS=
LP3D_CHECKS_FAIL=

# Applicatin status check (TODO - this command is throwing a segfault, check why)
#[ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
#${LPUB3D_EXE} ${LP3D_CHECK_STATUS} 2>/dev/null || true

${LPUB3D_EXE} ${LP3D_CHECK_STATUS} 2> ${LP3D_LOG_FILE}
if [ $? -ne 0 ];then
    echo "- LPub3D Status Log Trace..."
    cat "${LP3D_LOG_FILE}"
fi

if [ -n "${LP3D_CHECK_LDD}" ]; then
    echo && echo "-----------Library Dependencies-------------" && echo
    find ${LPUB3D_EXE} -executable -type f -exec ldd {} \;
fi

echo && echo "------------Build Checks Start--------------" && echo

for LP3D_BUILD_CHECK in CHECK01 CHECK02 CHECK03 CHECK04 CHECK05 CHECK06 CHECK07; do
    lp3d_check_start=$SECONDS
    case ${LP3D_BUILD_CHECK} in
    CHECK01)
        QT_DEBUG_PLUGINS=1
        LP3D_CHECK_LBL="Native File Process"
        LP3D_CHECK_HDR="- Check 1 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --liblego --preferred-renderer native"
        LP3D_LOG_FILE="Check.1.out"
        LP3D_CHECK_STDLOG=
        ;;
    CHECK02)
        LP3D_CHECK_LBL="LDView File Process"
        LP3D_CHECK_HDR="- Check 2 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer ldview"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        LP3D_LOG_FILE="Check.2.out"
        ;;
    CHECK03)
        LP3D_CHECK_LBL="LDView (Single Call) File Process"
        LP3D_CHECK_HDR="- Check 3 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer ldview-sc"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        LP3D_LOG_FILE="Check.3.out"
        ;;
    CHECK04)
        LP3D_CHECK_LBL="LDGLite Export Range"
        LP3D_CHECK_HDR="- Check 4 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stderr-ldglite"
        LP3D_LOG_FILE="Check.4.out"
        ;;
    CHECK05)
        LP3D_CHECK_LBL="Native POV Generation"
        LP3D_CHECK_HDR="- Check 5 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --liblego --preferred-renderer povray"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stderr-povray"
        LP3D_LOG_FILE="Check.5.out"
        ;;
    CHECK06)
        LP3D_CHECK_LBL="LDView TENTE Model"
        LP3D_CHECK_HDR="- Check 6 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --libtente --preferred-renderer ldview"
        LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/TENTE/astromovil.ldr"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        LP3D_LOG_FILE="Check.6.out"
        ;;
    CHECK07)
        LP3D_CHECK_LBL="LDView (Snapshot List) VEXIQ Model"
        LP3D_CHECK_HDR="- Check 7 of 7: ${LP3D_CHECK_LBL} Check..."
        LP3D_CHECK_OPTIONS="--no-stdout-log --process-file --clear-cache --libvexiq --preferred-renderer ldview-scsl"
        LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/VEXIQ/spider.mpd"
        LP3D_CHECK_STDLOG="${LP3D_CHECK_STDLOG}/stdout-ldview"
        LP3D_LOG_FILE="Check.7.out"
        ;;
      esac

    # Remove old log if exist
    if [ -f "${LP3D_LOG_FILE}" ]; then
        rm -rf "${LP3D_LOG_FILE}"
    fi

    # Initialize XVFB and execute check
    [ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} &> ${LP3D_LOG_FILE} || \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} &> ${LP3D_LOG_FILE}
    # allow some time between checks
    sleep 4
    # check output log for build check status
    if [ -f "${LP3D_LOG_FILE}" ]; then
        LP3D_ERROR=0
        if grep -q "${LP3D_CHECK_SUCCESS}" "${LP3D_LOG_FILE}"; then
            if grep -q "${LP3D_XVFB_ERROR}" "${LP3D_LOG_FILE}"; then
                LP3D_ERROR=1
            fi
            echo "${LP3D_CHECK_HDR} PASSED, ELAPSED TIME [`ElapsedCheckTime $lp3d_check_start`]"
            echo "${LP3D_CHECK_LBL} Command: ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}"
            let LP3D_CHECK_PASS++
            LP3D_CHECKS_PASS="${LP3D_CHECKS_PASS},$(echo ${LP3D_CHECK_HDR} | cut -d " " -f 3)"
            echo
        else
            LP3D_ERROR=2
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
        # Report Xvfb errors
        if [ "${LP3D_ERROR}" -eq "1" ]; then
            if grep -q "${LP3D_XVFB_ERROR}" "${LP3D_LOG_FILE}"; then
                echo "- LPub3D Xvfb Log Trace: ${LP3D_LOG_FILE}"
                cat "${LP3D_LOG_FILE}"
            fi
        fi
        if [[ "$LP3D_BUILD_OS" != "flatpak" && "$LP3D_BUILD_OS" != "snap" ]]; then
            rm -rf "${LP3D_LOG_FILE}"
        fi
    else
        echo "ERROR - ${LP3D_LOG_FILE} was not generated."
    fi
done

# remove leading ','
if [ "${LP3D_CHECK_PASS}" -gt "0" ]; then
    LP3D_CHECKS_PASS="$(echo ${LP3D_CHECKS_PASS} | cut -c 2-)"
fi
if [ "${LP3D_CHECK_FAIL}" -gt "0" ]; then
    LP3D_CHECKS_FAIL="$(echo ${LP3D_CHECKS_FAIL} | cut -c 2-)"
fi

SUMMARY_MSG=''
SUMMARY_MSG+="----Build Check Completed: PASS (${LP3D_CHECK_PASS})[${LP3D_CHECKS_PASS}], "
SUMMARY_MSG+="FAIL (${LP3D_CHECK_FAIL})[${LP3D_CHECKS_FAIL}], "
SUMMARY_MSG+="ELAPSED TIME [`ElapsedCheckTime $lp3d_elapsed_check_start`]----"
echo && echo $SUMMARY_MSG && echo
