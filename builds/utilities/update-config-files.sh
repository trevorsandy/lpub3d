#!/bin/bash
# Trevor SANDY
# Last Update September 20, 2024
# Copyright (C) 2016 - 2024 by Trevor SANDY
#
# This script is automatically executed by qmake from mainApp.pro
# It is also called by other config scripts accordingly
#
# To Run:
# cd <LPub3D root>
#
# env _PRO_FILE_PWD_=$PWD/mainApp LPUB3D=lpub3d OBS=true ./builds/utilities/update-config-files.sh
#
# Optional argument:
# _EXPORT_CONFIG_ONLY_ Do not update config files. Only export variables

LP3D_ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

echo "   Start $LP3D_ME execution from $PWD ($([ -z "${_EXPORT_CONFIG_ONLY_}" ] && echo Update config files || echo Skip config files update))..."

LP3D_CHANGE_DATE_LONG=`date +%a,\ %d\ %b\ %Y\ %H:%M:%S\ %z`
LP3D_CHANGE_DATE=`date +%a\ %b\ %d\ %Y`
LP3D_RELEASE_DATE=`date +%d.%m.%Y`
LP3D_DATE_TIME=`date +%d\ %m\ %Y\ %H:%M:%S`
LP3D_LAST_EDIT=`date +%d-%m-%Y`
LP3D_BUILD_DATE=`date "+%Y%m%d"`
LP3D_CALL_DIR=`pwd`
LP3D_OS=`uname`
LP3D_GIT_DEPTH=150000
LP3D_CMD_COUNT=0

if [ -z "$1" ]; then SOURCED="true"; LP3D_PWD=${_PRO_FILE_PWD_}; else SOURCED="false"; LP3D_PWD=$1; fi
cd $LP3D_PWD/.. && basedir=$PWD && cd $LP3D_CALL_DIR

# Change these when you change the LPub3D root directory (e.g. if using a different root folder when testing)
[ -z "${LPUB3D}" ] && LPUB3D=$(basename "$(echo "$basedir")") || true

Info () {
    if [ "${SOURCED}" = "true" ]
    then
        f="${0##*/}"; f="${f%.*}"
        echo "   ${f}: ${*}" >&2
    else
        echo "${*}" >&2
    fi
}

# Fake realpath
realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH_="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH_"
}

# Change these accordingly when respective config files are modified
if [ "$OBS" = "true" ]
then
    UPDATE_OBS_ALL_DEPS=Yes
    LP3D_CONFIG_DIR=$(realpath "$LP3D_PWD/../builds/linux/obs/alldeps")
else
    UPDATE_OBS_ALL_DEPS=No
    LP3D_CONFIG_DIR=$(realpath "$LP3D_PWD/../builds/linux/obs")
fi
LP3D_UTIL_DIR=$(realpath "$LP3D_PWD/../builds/utilities")

if [ "$LP3D_PWD" = "" ] && [ "${_PRO_FILE_PWD_}" = "" ]
then
    Info "Error: Did not receive required argument _PRO_FILE_PWD_"
    Info "$LP3D_ME terminated!"
    exit 1
fi

if [ "${SOURCED}" != "true" ]
then
    # logging stuff
    [ -z "${LP3D_LOG_PATH}" ] && LP3D_LOG_PATH=$LP3D_CALL_DIR || :
    LOG="$LP3D_LOG_PATH/$LP3D_ME.log"
    if [ -f ${LOG} -a -r ${LOG} ]
    then
        rm ${LOG}
    fi
    exec > >(tee -a ${LOG} )
    exec 2> >(tee -a ${LOG} >&2)
fi

#Info "   DEBUG INPUT ARGS \$0 [$0], \$1 [$1], \$2 [$2], \$3 [$3], \$4 [$4], \$5 [$5], \$6 [$6], \$7 [$7], \$8 [$8]"
if [ "${SOURCED}" = "true" ]
then
    cd "$(realpath $LP3D_PWD/..)"
    if [ "${CI}" = "true" ];
    then
        # Update refs and tags and populate committer email, name
        Info "$((LP3D_CMD_COUNT += 1)). update git tags and capture version info using git queries"
        if [ ! -d ".git" ]; then
            Info "ERROR - No .git folder in $PWD"
        fi
        if [ -n "$TRAVIS" ]; then
            LP3D_BRANCH=${TRAVIS_BRANCH}
            LP3D_COMMIT=${TRAVIS_COMMIT}
        elif [ -n "$GITHUB" ]; then
            LP3D_BRANCH=${GITHUB_REF}
            LP3D_COMMIT=${GITHUB_SHA}
        fi
        git fetch -qfup --depth=${LP3D_GIT_DEPTH} origin +${LP3D_BRANCH} +refs/tags/*:refs/tags/* >/dev/null 2>&1
        git checkout -qf ${LP3D_COMMIT} >/dev/null 2>&1
        lp3d_git_ver_author="$(git log -1 ${LP3D_COMMIT} --pretty="%aN")" >/dev/null 2>&1
        lp3d_git_ver_committer_email="$(git log -1 ${LP3D_COMMIT} --pretty="%cE")" >/dev/null 2>&1
    else
        Info "$((LP3D_CMD_COUNT += 1)). capture version info using git queries"
    fi
    lp3d_git_build_type=`git tag --points-at HEAD` >/dev/null 2>&1                     # continuous build check
    lp3d_git_ver_tag_long=`git describe --tags --match v* --long` >/dev/null 2>&1
    lp3d_git_ver_tag_short=`git describe --tags --match v* --abbrev=0` >/dev/null 2>&1
    lp3d_git_ver_commit_count=`git rev-list --count HEAD` >/dev/null 2>&1
    lp3d_git_ver_sha_hash_short=`git rev-parse --short HEAD` >/dev/null 2>&1
    cd "${LP3D_CALL_DIR}"
    lp3d_ver_tmp=${lp3d_git_ver_tag_long#*-}                                          # remove everything before and including "-"
    lp3d_revision_=${lp3d_ver_tmp%-*}                                                 # remove everything after and including "-"
    lp3d_ver_tmp=${lp3d_git_ver_tag_short//./" "}                                     # replace . with " "
    lp3d_version_=${lp3d_ver_tmp/v/}                                                  # replace v with ""
    lp3d_ver_tmp=${lp3d_version_#*_}                                                  # remove everything before and including "_" if exist
    if test -z "$lp3d_git_build_type"; then LP3D_BUILD_TYPE="Continuous"; else LP3D_BUILD_TYPE="Release"; fi
    if test "$lp3d_ver_tmp" != "$lp3d_version_"; then lp3d_suffix=${lp3d_ver_tmp}; fi # check if ver_tmp not same as version_ - suffix exist
    if test -n "$lp3d_suffix"; then lp3d_version_=${lp3d_version_%_*}; fi             # remove everything after and including "_" - suffix exist
    if test -n "$lp3d_git_ver_author"; then LP3D_AUTHOR_NAME=${lp3d_git_ver_author}; else LP3D_AUTHOR_NAME=`echo $USER`; fi
    if test -n "$lp3d_git_ver_committer_email"; then LP3D_COMMITTER_EMAIL=${lp3d_git_ver_committer_email}; else LP3D_COMMITTER_EMAIL=undefined; fi
    LP3D_VERSION_INFO=${lp3d_version_}" "${lp3d_revision_}" "${lp3d_git_ver_commit_count}" "${lp3d_git_ver_sha_hash_short}
else
    Info "$((LP3D_CMD_COUNT += 1)). capture version info using input arguments"
    LP3D_BUILD_TYPE="Default"
    if test -n "$8"; then lp3d_suffix=$8; fi
    LP3D_VERSION_INFO=$2" "$3" "$4" "$5" "$6" "$7
fi
if test -n "$lp3d_suffix"; then
    LP3D_VERSION_INFO="$LP3D_VERSION_INFO $lp3d_suffix"
fi
#         1 2 3  4  5   6
# format "2 0 20 17 663 410fdd7"
read LP3D_VER_MAJOR LP3D_VER_MINOR LP3D_VER_PATCH LP3D_VER_REVISION LP3D_VER_BUILD LP3D_VER_SHA_HASH LP3D_VER_SUFFIX THE_REST <<< ${LP3D_VERSION_INFO//'"'}
LP3D_APP_VER_SUFFIX=${LP3D_VER_MAJOR}${LP3D_VER_MINOR}
LP3D_VERSION=${LP3D_VER_MAJOR}"."${LP3D_VER_MINOR}"."${LP3D_VER_PATCH}
LP3D_BUILD_VERSION=${LP3D_VERSION}"."${LP3D_VER_REVISION}"."${LP3D_VER_BUILD}" ("${LP3D_DATE_TIME}")"
LP3D_APP_VERSION=${LP3D_VERSION}"."${LP3D_VER_BUILD}
LP3D_APP_VERSION_LONG=${LP3D_VERSION}"."${LP3D_VER_REVISION}"."${LP3D_VER_BUILD}_${LP3D_BUILD_DATE}
LP3D_APP_VERSION_TAG="v"${LP3D_VERSION}

Info "   LP3D_BUILD_TYPE........${LP3D_BUILD_TYPE}"
Info "   LPUB3D_DIR.............${LPUB3D}"
if test "${CI}" = "true"; then
Info "   CI.....................${CI}"
[ -n "${GITHUB}" ] && Info "   GITHUB.................${GITHUB}" || :
[ -n "${LP3D_BRANCH}" ] && Info "   LP3D_BRANCH............${LP3D_BRANCH}" || :
[ -n "${LP3D_COMMIT}" ] && Info "   LP3D_COMMIT............${LP3D_COMMIT}" || :
fi
Info "   UPDATE_OBS_ALL_DEPS....${UPDATE_OBS_ALL_DEPS}"

Info "   LP3D_PWD...............${LP3D_PWD}"
Info "   LP3D_CALL_DIR..........${LP3D_CALL_DIR}"

Info "   LP3D_VER_MAJOR.........${LP3D_VER_MAJOR}"
Info "   LP3D_VER_MINOR.........${LP3D_VER_MINOR}"
Info "   LP3D_VER_PATCH.........${LP3D_VER_PATCH}"
Info "   LP3D_VER_REVISION......${LP3D_VER_REVISION}"
Info "   LP3D_VER_BUILD.........${LP3D_VER_BUILD}"
Info "   LP3D_VER_SHA_HASH......${LP3D_VER_SHA_HASH}"
if test -n "$LP3D_VER_SUFFIX"; then
Info "   LP3D_VER_SUFFIX........${LP3D_VER_SUFFIX}"
fi
Info "   LP3D_VERSION_INFO......${LP3D_VERSION_INFO}"
Info "   LP3D_APP_VERSION.......${LP3D_APP_VERSION}"
Info "   LP3D_APP_VERSION_LONG..${LP3D_APP_VERSION_LONG}"
# Info "   LP3D_APP_VERSION_TAG...${LP3D_APP_VERSION_TAG}"
Info "   LP3D_APP_VER_SUFFIX....${LP3D_APP_VER_SUFFIX}"
Info "   LP3D_DATE_TIME.........${LP3D_DATE_TIME}"
Info "   LP3D_RELEASE_DATE......${LP3D_RELEASE_DATE}"
Info "   LP3D_CHANGE_DATE_LONG..${LP3D_CHANGE_DATE_LONG}"

Info "   LP3D_VERSION...........${LP3D_VERSION}"
Info "   LP3D_BUILD_VERSION.....${LP3D_BUILD_VERSION}"

Info "   LP3D_SOURCE_DIR........${LPUB3D}-${LP3D_APP_VERSION}"
if test -n "${LP3D_AUTHOR_NAME}"; then
Info "   LP3D_AUTHOR_NAME.......${LP3D_AUTHOR_NAME}"
fi
if test -n "${LP3D_COMMITTER_EMAIL}"; then
Info "   LP3D_COMMITTER_EMAIL...${LP3D_COMMITTER_EMAIL}"
fi

LP3D_NO_CONFIG_DISPLAY=$(if [[ "${CI}" = "true" || "${GITHUB}" = "true" ]]; then echo "1"; fi)
if [ -n "${LP3D_NO_CONFIG_DISPLAY}" ]; then
    export LP3D_VER_MAJOR=${LP3D_VER_MAJOR}
    export LP3D_VER_MINOR=${LP3D_VER_MINOR}
    export LP3D_VER_PATCH=${LP3D_VER_PATCH}
    export LP3D_VER_REVISION=${LP3D_VER_REVISION}
    export LP3D_VER_BUILD=${LP3D_VER_BUILD}
    export LP3D_VER_SHA_HASH=${LP3D_VER_SHA_HASH}
    export LP3D_RELEASE_DATE=${LP3D_RELEASE_DATE}
    test -n "$LP3D_VER_SUFFIX" && export LP3D_VER_SUFFIX=$LP3D_VER_SUFFIX || :
    export LP3D_VERSION=${LP3D_VERSION}
    export LP3D_VERSION_INFO=${LP3D_VERSION_INFO}
    export LP3D_APP_VERSION_LONG=${LP3D_APP_VERSION_LONG}
    export LP3D_APP_VERSION_TAG=${LP3D_APP_VERSION_TAG}
    export LP3D_COMMITTER_EMAIL=${LP3D_COMMITTER_EMAIL}
    export LP3D_AUTHOR_NAME=${LP3D_AUTHOR_NAME}
    export LP3D_BUILD_TYPE=${LP3D_BUILD_TYPE}
    export LP3D_BUILD_VERSION=${LP3D_BUILD_VERSION}
fi

# generate version.info file
FILE="$LP3D_UTIL_DIR/version.info"
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    rm ${FILE}
fi
cat <<EOF >${FILE}
${LP3D_VERSION_INFO}
EOF

if [ -f "${FILE}" ];
then
    Info "$((LP3D_CMD_COUNT += 1)). create version.info    - insert version info   [$FILE]";
else
    Info "   ERROR - version info   - file not found";
fi

# ..............................
if [ -z "${_EXPORT_CONFIG_ONLY_}" ]; then # _EXPORT_CONFIG_ONLY_ not set, Update config files
# ..............................

if [ "$LP3D_OS" = Darwin ]
then
    [ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
    Info "$((LP3D_CMD_COUNT += 1)). update the Info.plist with version major, version minor, build and git sha hash" || :
    if test -n "$LP3D_VER_SUFFIX"; then
        LP3D_BUNDLE_VERSION=${LP3D_VER_BUILD}-${LP3D_VER_SUFFIX}
    fi
    LP3D_INFO_PLIST_FILE="$LP3D_PWD/Info.plist"
    if [ -f "${LP3D_INFO_PLIST_FILE}" ]
    then
        /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString ${LP3D_VERSION}" "${LP3D_INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :CFBundleVersion ${LP3D_VERSION}.${LP3D_BUNDLE_VERSION}-${LP3D_VER_SHA_HASH}" "${LP3D_INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :CFBundleGetInfoString LPub3D ${LP3D_VERSION} https://github.com/trevorsandy/${LPUB3D}" "${LP3D_INFO_PLIST_FILE}"
        /usr/libexec/PlistBuddy -c "Set :com.trevorsandy.lpub3d.GitSHA ${LP3D_VER_SHA_HASH}" "${LP3D_INFO_PLIST_FILE}"
    else
        Info "   Error: update failed, ${LP3D_INFO_PLIST_FILE} not found."
    fi
fi

FILE="$LP3D_PWD/docs/RELEASE_NOTES.html"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)). update RELEASE_NOTES   - build version         [$FILE]" || :
LineToReplace=20
StringToReplace="      <h4><a id=\"LPub3D_0\"></a>LPub3D ${LP3D_BUILD_VERSION}</h4>"
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s;.*;${StringToReplace};" "${FILE}"
    else
        sed -i "${LineToReplace}s;.*;${StringToReplace};" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$LP3D_PWD/docs/README.txt"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)). update README.txt      - build version         [$FILE]" || :
LineToReplace=1                  # LPub3D 2.0.21.59.126...
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" "${LineToReplace}s/.*/LPub3D ${LP3D_BUILD_VERSION}/" "${FILE}"
    else
        sed -i "${LineToReplace}s/.*/LPub3D ${LP3D_BUILD_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$(realpath $LP3D_PWD/../README.md)"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)). update README.md       - update last edit date [$FILE]" || :
LastEdit="\[gh-maintained-url\]: https:\/\/github.com\/trevorsandy\/lpub3d\/projects\/1 \"Last edited ${LP3D_LAST_EDIT}\""
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" -e "s/^\[gh-maintained-url]:.*/${LastEdit}/" "${FILE}"
    else
        sed -i    -e "s/^\[sfreleases\]:.*/${SFReplacement}\/${LP3D_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$LP3D_PWD/lpub3d.desktop"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)). update desktop config  - add version suffix    [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" "s/^Exec=lpub3d.*/Exec=lpub3d${LP3D_APP_VER_SUFFIX} %f/" "${FILE}"
    else
        sed -i "s/^Exec=lpub3d.*/Exec=lpub3d${LP3D_APP_VER_SUFFIX} %f/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$LP3D_PWD/lpub3d.appdata.xml"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)). update appdata info    - update binary version [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" -e "s/.*<binary>lpub3d.*/            <binary>lpub3d${LP3D_APP_VER_SUFFIX}<\/binary>/" "${FILE}"

    else
        sed -i -e "s/.*<binary>lpub3d.*/            <binary>lpub3d${LP3D_APP_VER_SUFFIX}<\/binary>/" "${FILE}" "${FILE}"
        # perform release update when the last commit is an annotated tag else perform a continuous build update
        last_commit_sha=$(cd $LP3D_PWD/.. && git rev-parse HEAD) >/dev/null 2>&1
        last_annotated_tag_sha=$(cd $LP3D_PWD/.. && git rev-list -n 1 $(git describe --abbrev=0)) >/dev/null 2>&1
        Info "$((LP3D_CMD_COUNT += 1)). update appdata info    - add version and date  [$FILE]" || :
        sed -i -e "0,/.*<release version=.*/{s/.*<release version=.*/            <release version=\"${LP3D_APP_VERSION}\" date=\"$(date "+%Y-%m-%d")\">/}" "${FILE}"
        if [ "${last_commit_sha}" = "${last_annotated_tag_sha}" ]; then this_is_a_release_build=1; fi
        Info "   update appdata release - compare last sha hash [commit: ${last_commit_sha:0:8}, annotated tag: ${last_annotated_tag_sha:0:8}]"
        if [ -n "$this_is_a_release_build" ]
        then
            sed -i -e "0,/.*<p>LPub3D.*/{s/.*<p>LPub3D.*/                    <p>LPub3D $(date "+%d.%m.%Y") v${LP3D_VERSION} Release Build./}" "${FILE}"
        else
            sed -i -e "0,/.*<p>LPub3D.*/{s/.*<p>LPub3D.*/                    <p>LPub3D $(date "+%d.%m.%Y") v${LP3D_VERSION} r${LP3D_VER_REVISION} Continuous Build./}" "${FILE}"
        fi
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$LP3D_PWD/docs/lpub3d${LP3D_APP_VER_SUFFIX}.1"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)). update man page        - add version suffix    [$FILE]" || :
FILE_TEMPLATE=`ls $LP3D_PWD/docs/lpub3d.*` >/dev/null 2>&1
if [ -f ${FILE_TEMPLATE} ];
then
    if [ -f ${FILE} ];
    then
        rm -f "${FILE}"
    fi
    cp "${FILE_TEMPLATE}" "${FILE}"
fi
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" "s/     \/usr\/bin\/lpub3d.*/     \/usr\/bin\/lpub3d${LP3D_APP_VER_SUFFIX}/" "${FILE}"
    else
        sed -i "s/     \/usr\/bin\/lpub3d.*/     \/usr\/bin\/lpub3d${LP3D_APP_VER_SUFFIX}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} (be sure ${FILE_TEMPLATE} exsit) from ${LP3D_CALL_DIR}"
fi

FILE="$LP3D_CONFIG_DIR/debian/changelog"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)).create changelog       - add version and date  [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    rm ${FILE}
fi
cat <<EOF >${FILE}
${LPUB3D} (${LP3D_APP_VERSION}) debian; urgency=medium

  * LPub3D version ${LP3D_APP_VERSION_LONG} for Linux

 -- Trevor SANDY <trevor.sandy@gmail.com>  ${LP3D_CHANGE_DATE_LONG}
EOF

FILE="$LP3D_CONFIG_DIR/PKGBUILD"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)).update PKGBUILD        - add version           [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" -e "s/^# Last Update:.*/# Last Update: $(date "+%B %d, %Y")/g" \
                  -e "s/^pkgver.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i -e "s/^# Last Update:.*/# Last Update: $(date "+%B %d, %Y")/g" \
               -e "s/^pkgver.*/pkgver=${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$LP3D_CONFIG_DIR/${LPUB3D}.spec"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)).update ${LPUB3D}.spec  - add version and date  [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" -e "s/^# Last Update:.*/# Last Update: $(date "+%B %d, %Y")/g" \
                  -e "s/^Version:.*/Version: ${LP3D_APP_VERSION}/" \
                  -e "s/.*trevor\.dot\.sandy\.at\.gmail\.dot\.com.*/* ${LP3D_CHANGE_DATE} - trevor\.dot\.sandy\.at\.gmail\.dot\.com ${LP3D_APP_VERSION}/" "${FILE}"

    else
        sed -i -e "s/^# Last Update:.*/# Last Update: $(date "+%B %d, %Y")/g" \
               -e "s/^Version:.*/Version: ${LP3D_APP_VERSION}/" \
               -e "s/.*trevor\.dot\.sandy\.at\.gmail\.dot\.com.*/* ${LP3D_CHANGE_DATE} - trevor\.dot\.sandy\.at\.gmail\.dot\.com ${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$LP3D_CONFIG_DIR/debian/${LPUB3D}.dsc"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)).update ${LPUB3D}.dsc   - add version           [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" "s/^Version:.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
    else
        sed -i "s/^Version:.*/Version: ${LP3D_APP_VERSION}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$(realpath $LP3D_PWD/../builds/linux/obs/debian/rules)"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)).update debian rules    - add version suffix    [$FILE]" || :
LP3D_OS_ARCH=32 && \
[[ "$(uname -m)" = "x86_64" || "$(uname -m)" = "aarch64" ]] && \
LP3D_OS_ARCH=64
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" -e "s/^# Last Update:.*/# Last Update: $(date "+%B %d, %Y")/g" \
                  -e "s/validExe/mainApp\/${LP3D_OS_ARCH}bit_release\/lpub3d${LP3D_APP_VER_SUFFIX}/" "${FILE}"
    else
        sed -i -e "s/^# Last Update:.*/# Last Update: $(date "+%B %d, %Y")/g" \
               -e "s/validExe/mainApp\/${LP3D_OS_ARCH}bit_release\/lpub3d${LP3D_APP_VER_SUFFIX}/" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$(realpath $LP3D_PWD/../gitversion.pri)"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)).update gitversion pri  - add version and revision [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    if test -n "$LP3D_VER_SUFFIX"; then
        LP3D_DOT_VER_SUFFIX=".${LP3D_VER_SUFFIX}"
    fi
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" -e "s/^VERSION = [0-9].*$/VERSION = ${LP3D_VERSION}/g" \
                  -e "s/^        GIT_REVISION = [0-9].*$/        GIT_REVISION = ${LP3D_VER_REVISION}/g" \
                  -e "s/^            GIT_COMMIT = [0-9].*$/            GIT_COMMIT = ${LP3D_VER_BUILD}/g" \
                  -e "s/^            GIT_VERSION = \$\${VERSION}.[0-9].*$/            GIT_VERSION = \$\$\{VERSION\}\.${LP3D_VER_REVISION}\.${LP3D_VER_BUILD}\.${LP3D_VER_SHA_HASH}${LP3D_DOT_VER_SUFFIX}/g" "${FILE}"
    else
        sed -i -e "s/^VERSION = [0-9].*$/VERSION = ${LP3D_VERSION}/g" \
               -e "s/^        GIT_REVISION = [0-9].*$/        GIT_REVISION = ${LP3D_VER_REVISION}/g" \
               -e "s/^            GIT_COMMIT = [0-9].*$/            GIT_COMMIT = ${LP3D_VER_BUILD}/g" \
               -e "s/^            GIT_VERSION = \$\${VERSION}.[0-9].*$/            GIT_VERSION = \$\$\{VERSION\}\.${LP3D_VER_REVISION}\.${LP3D_VER_BUILD}\.${LP3D_VER_SHA_HASH}${LP3D_DOT_VER_SUFFIX}/g" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

FILE="$(realpath $LP3D_PWD/../mainApp/extras/LPub3D_Npp_UDL.xml)"
[ -z "$LP3D_NO_CONFIG_DISPLAY" ] && \
Info "$((LP3D_CMD_COUNT += 1)).update Notepad-pp UDL  - update version and date  [$FILE]" || :
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    LP3D_UDL_APP_VERSION="${LP3D_VERSION}.${LP3D_VER_REVISION}.${LP3D_VER_BUILD}"
    if [ "$LP3D_OS" = Darwin ]
    then
        sed -i "" -e "s/^;; Version.....:.*/;; Version.....: ${LP3D_UDL_APP_VERSION}/" \
                  -e "s/^;; Last Update.:.*/;; Last Update.: ${LP3D_CHANGE_DATE_LONG}/g" "${FILE}"
    else
        sed -i -e "s/^;; Version.....:.*/;; Version.....: ${LP3D_UDL_APP_VERSION}/" \
               -e "s/^;; Last Update.:.*/;; Last Update.: ${LP3D_CHANGE_DATE_LONG}/g" "${FILE}"
    fi
else
    Info "   Error: Cannot read ${FILE} from ${LP3D_CALL_DIR}"
fi

# ..............................
else # _EXPORT_CONFIG_ONLY_ set, skip config files update
    Info "   Skip config files update (Config on export only)."
fi
# ..............................

if [ "${SOURCED}" = "false" ]
then
    Info "   Script $LP3D_ME execution finshed."
fi
echo
