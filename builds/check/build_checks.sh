#!/bin/bash
# Trevor SANDY
# Last Update July 10, 2018
# LPub3D Unix checks
# NOTE: this file must be sourced. with ${LPUB3D_EXE} and ${SOURCE_DIR} predefined.

if [[ "${DOCKER}" = "true" && ${XMING} != "true" ]]; then
echo && echo "- Using XVFB"
USE_XVFB="true"
fi

LP3D_CHECK_FILE="$(realpath ${SOURCE_DIR})/builds/check/build_checks.mpd"

for LP3D_BUILD_CHECK in CHECK01 CHECK02 CHECK03 CHECK04; do
    case ${LP3D_BUILD_CHECK} in
    CHECK01)
        LP3D_CHECK_HDR="- Check 1 of 4: Native File Process Check..."
        LP3D_CHECK_OPTIONS="--process-file --preferred-renderer native"
        ;;
    CHECK02)
        LP3D_CHECK_HDR="- Check 2 of 4: LDView File Process Check..."
        LP3D_CHECK_OPTIONS="--process-file --clear-cache --preferred-renderer ldview"
        ;;
    CHECK03)
        LP3D_CHECK_HDR="- Check 3 of 4: LDGLite Export Range Check..."
        LP3D_CHECK_OPTIONS="--process-export --range 1-3 --clear-cache --preferred-renderer ldglite"
        ;;
    CHECK04)
        LP3D_CHECK_HDR="- Check 4 of 4: Native POV Generation Check..."
        LP3D_CHECK_OPTIONS="--process-file --clear-cache --preferred-renderer povray"
        ;;
      esac

    echo && echo ${LP3D_CHECK_HDR}
    [ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} || \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}
done

echo "------------Build checks completed----------" && echo

