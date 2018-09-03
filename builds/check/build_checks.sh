#!/bin/bash
# Trevor SANDY
# Last Update August 10, 2018
# LPub3D Unix checks - for remote CI (Trevis, OBS) 
# NOTE: Source with variables as appropriate:
#       $LP3D_COMPILE_SOURCE = true,
#       $LP3D_BUILD_APPIMAGE = true,
#       $SOURCE_DIR = <lpub3d source folder>
#       $LP3D_DIST_DIR_PATH = 3rdParty library path
#       Set OBS=false when building locally

# Initialize platform variables
LP3D_OS_NAME=$(uname)
LP3D_TARGET_ARCH="$(uname -m)"
LP3D_RELEASE="32bit_release"
if [[ "${LP3D_TARGET_ARCH}" = "x86_64" || "${LP3D_TARGET_ARCH}" = "aarch64" ]] ; then
    LP3D_RELEASE="64bit_release"
fi

# Initialize XVFB 
if [[ "${XMING}" != "true" && ("${DOCKER}" = "true" || "${LP3D_OS_NAME}" != "Darwin") ]]; then
    echo && echo "- Using XVFB from working directory: ${PWD}"
    USE_XVFB="true"
fi

# Initialize OBS if not in command line input
if [[ "${OBS}" = "" && "${DOCKER}" = "" &&  "${TRAVIS}" = "" ]]; then
  LP3D_OBS=true
fi

# Initialize build paths and libraries
if [[ "${LP3D_OS_NAME}" = "Darwin" && "$BUILD_OPT" = "compile" ]]; then
    cd ${SOURCE_DIR}/mainApp/${LP3D_RELEASE}
    
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
    
elif [[ ("${TRAVIS_OS_NAME}" = "linux" && "${LP3D_COMPILE_SOURCE}" = "true") || \
        ("${LP3D_OBS}" = "true" && "${LP3D_BUILD_APPIMAGE}" != "true") || \
        ("${DOCKER}" = "true" && -z "${LP3D_INSTALL_PKG}") ]]; then
    cd ${SOURCE_DIR}
    
    echo "- set Linux ${LP3D_BUILD_ARCH} LPub3D executable..."
    LPUB3D_VER="lpub3d${LP3D_APP_VER_SUFFIX}"
    LPUB3D_EXE="mainApp/${LP3D_RELEASE}/${LPUB3D_VER}"

    echo "- set 3rdParty renderer paths..." # 3rdParty renderer paths
    LP3D_3RD_LDGLITE="${LP3D_DIST_DIR_PATH}/${LP3D_VER_LDGLITE}"
    LP3D_3RD_LDVIEW="${LP3D_DIST_DIR_PATH}/${LP3D_VER_LDVIEW}"
    LP3D_3RD_POVRAY="${LP3D_DIST_DIR_PATH}/${LP3D_VER_POVRAY}"
	
    # 3rdParty renderer exe paths
    LP3D_3RD_LDGLITE_EXE="${LP3D_3RD_LDGLITE}/bin/${LP3D_TARGET_ARCH}/ldglite"
    LP3D_3RD_LDVIEW_EXE="${LP3D_3RD_LDVIEW}/bin/${LP3D_TARGET_ARCH}/ldview"
    LP3D_3RD_POVRAY_EXE="${LP3D_3RD_POVRAY}/bin/${LP3D_TARGET_ARCH}/lpub3d_trace_cui"
	
    echo "- setup share contents..."         # mainApp/share
    LP3D_3RD_SHARE="mainApp/share/${LPUB3D_VER}/3rdParty"
    LP3D_LDGLITE_SHARE="${LP3D_3RD_SHARE}/${LP3D_VER_LDGLITE}"
    LP3D_LDVIEW_SHARE="${LP3D_3RD_SHARE}/${LP3D_VER_LDVIEW}"
    LP3D_POVRAY_SHARE="${LP3D_3RD_SHARE}/${LP3D_VER_POVRAY}"
    mkdir -p ${LP3D_LDGLITE_SHARE}
    mkdir -p ${LP3D_LDVIEW_SHARE}
    mkdir -p ${LP3D_POVRAY_SHARE}
    cp -rf ${LP3D_3RD_LDGLITE}/docs ${LP3D_LDGLITE_SHARE}/
    cp -rf ${LP3D_3RD_LDGLITE}/resources ${LP3D_LDGLITE_SHARE}/
    cp -rf ${LP3D_3RD_LDVIEW}/docs ${LP3D_LDVIEW_SHARE}/
    cp -rf ${LP3D_3RD_LDVIEW}/resources ${LP3D_LDVIEW_SHARE}/
    cp -rf ${LP3D_3RD_POVRAY}/docs ${LP3D_POVRAY_SHARE}/
    cp -rf ${LP3D_3RD_POVRAY}/resources ${LP3D_POVRAY_SHARE}/
	
    echo "- setup extras contents..." # mainApp/extras
    cp -rf mainApp/extras mainApp/share/${LPUB3D_VER}/
	
    echo "- setup 3rdParty bin contents..." # opt
    LP3D_OPT_BIN="opt/${LPUB3D_VER}/3rdParty"
    LP3D_LDGLITE_BIN="${LP3D_OPT_BIN}/${LP3D_VER_LDGLITE}/bin"
    LP3D_LDVIEW_BIN="${LP3D_OPT_BIN}/${LP3D_VER_LDVIEW}/bin"
    LP3D_POVRAY_BIN="${LP3D_OPT_BIN}/${LP3D_VER_POVRAY}/bin"
    mkdir -p ${LP3D_LDGLITE_BIN}
    mkdir -p ${LP3D_LDVIEW_BIN}
    mkdir -p ${LP3D_POVRAY_BIN}
    cp -rf ${LP3D_3RD_LDGLITE_EXE} ${LP3D_LDGLITE_BIN}/
    cp -rf ${LP3D_3RD_LDVIEW_EXE} ${LP3D_LDVIEW_BIN}/
    cp -rf ${LP3D_3RD_POVRAY_EXE} ${LP3D_POVRAY_BIN}/

    echo "- update LD_LIBRARY_PATH and set Qt base..."  #Qt libs, ldrawini and quazip
    LP3D_QUAZIP_LIB=$(realpath quazip/${LP3D_RELEASE}/)
    LP3D_LDRAWINI_LIB=$(realpath ldrawini/${LP3D_RELEASE}/)
    export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${LP3D_QUAZIP_LIB}:${LP3D_LDRAWINI_LIB}"
    if [ "${TRAVIS}" = "true" ]; then
       source /opt/qt${LP3D_QT_BASE}/bin/qt${LP3D_QT_BASE}-env.sh
    fi
	
    echo "- run LDD: ${LPUB3D_EXE}..."
    if [ -f "${LPUB3D_EXE}" ]; then
        ldd ${LPUB3D_EXE} 2>/dev/null
    else
        echo "ERROR - LDD check failed for $(realpath ${LPUB3D_EXE})"
    fi
elif [ "$LP3D_BUILD_APPIMAGE" = "true" ]; then
    cd ${SOURCE_DIR}
     
    echo "- set $(realpath ${LPUB3D_EXE}) execute permissions..."
    [ -f "${LPUB3D_EXE}" ] && \
    chmod u+x ${LPUB3D_EXE} || \
    echo "ERROR - $(realpath ${LPUB3D_EXE}) not found." 
fi

if [[ ("${TRAVIS_OS_NAME}" = "linux" && "${LP3D_COMPILE_SOURCE}" = "true") || \
      ("${LP3D_OBS}" = "true" && "${LP3D_BUILD_APPIMAGE}" != "true") || \
      ("${DOCKER}" = "true" && -z "${LP3D_INSTALL_PKG}") ]]; then
    echo "- ldraw libraries..."
    if [ ! -f "mainApp/extras/complete.zip" ]
    then
       if [ -f "${LP3D_DIST_DIR_PATH}/complete.zip" ]
       then
         echo "- copy ldraw library archives to $(realpath mainApp/share/${LPUB3D_VER})..."
         cp -f "${LP3D_DIST_DIR_PATH}/complete.zip" "mainApp/share/${LPUB3D_VER}/complete.zip"
         cp -f "${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip" "mainApp/share/${LPUB3D_VER}/lpub3dldrawunf.zip"
       else
         echo "- download ldraw official library archive to ${LP3D_DIST_DIR_PATH}..."
         curl -sL -C - http://www.ldraw.org/library/updates/complete.zip -o ${LP3D_DIST_DIR_PATH}/complete.zip
         echo "- download ldraw unofficial library archive to ${LP3D_DIST_DIR_PATH}..."
         curl -sL -C - http://www.ldraw.org/library/unofficial/ldrawunf.zip -o ${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip
         echo "- copy ldraw library archive to $(realpath mainApp/share/${LPUB3D_VER})..."
         cp -f "${LP3D_DIST_DIR_PATH}/complete.zip" "mainApp/share/${LPUB3D_VER}/complete.zip"
         cp -f "${LP3D_DIST_DIR_PATH}/lpub3dldrawunf.zip" "mainApp/share/${LPUB3D_VER}/lpub3dldrawunf.zip"
       fi
    else
        echo "- copy ldraw library archives to $(realpath mainApp/share/${LPUB3D_VER})..."
        cp -f "mainApp/extras/complete.zip" "mainApp/share/${LPUB3D_VER}/complete.zip"
        cp -f "mainApp/extras/lpub3dldrawunf.zip" "mainApp/share/${LPUB3D_VER}/lpub3dldrawunf.zip"
    fi
fi

echo && echo "------------Build checks start--------------" && echo

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
    #echo "  Executable: $(realpath ${LPUB3D_VER})"
    [ -n "$USE_XVFB" ] && xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE} || \
    ${LPUB3D_EXE} ${LP3D_CHECK_OPTIONS} ${LP3D_CHECK_FILE}
done

echo && echo "------------Build checks completed----------" && echo

