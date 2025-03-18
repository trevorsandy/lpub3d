#!/bin/bash
# Trevor SANDY
# Last Update: March 18, 2025
#
# ToRun: (cd ~/projects/lpub3d && ../build_commit.sh)
#        (cd ~/projects/lpub3d && env COMMIT="LPub3D continuous development" ../build_commit.sh)
#        (cd ~/projects/lpub3d && env COMMIT="LPub3D update [skip ci]" ../build_commit.sh)
#        (cd ~/projects/lpub3d && env COMMIT="LPub3D v2.4.9" ../build_commit.sh)
#
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
SCRIPTDIR=$(dirname "$0")
f="${SCRIPTDIR}/$ME"
f="${f%.*}"
ext=".log"
if [[ -e "${f}_0${ext}" ]]
then
    i=1
    while [[ -e "${f}_${i}${ext}" ]]
    do
      let i++
    done
    f="${f}_${i}${ext}"
else
    f="${f}_0${ext}"
fi
# output log file
LOG="$f"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)
COMMAND_COUNT=0
COMMANDS=6

COMMIT_MSG=${COMMIT:-LPub3D continuous release_build}

echo "$((COMMAND_COUNT += 1)) of ${COMMANDS} - checkout master branch"
if [ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]; then git checkout master; fi

echo "$((COMMAND_COUNT += 1)) of ${COMMANDS} - Change line endings from CRLF to LF"
dos2unix -k builds/utilities/ci/github/* &> $LOG
dos2unix -k builds/utilities/ci/secure/* &>> $LOG
dos2unix -k builds/utilities/ci/travis/* &>> $LOG
dos2unix -k builds/utilities/ci/ci_cutover.sh &>> $LOG
dos2unix -k builds/utilities/ci/next_cutover.sh &>> $LOG
dos2unix -k builds/utilities/ci/sfdeploy.sh &>> $LOG
dos2unix -k builds/utilities/dmg-utils/support/* &>> $LOG
dos2unix -k builds/utilities/dmg-utils/* &>> $LOG
dos2unix -k builds/utilities/hooks/* &>> $LOG
dos2unix -k builds/utilities/json/* &>> $LOG
dos2unix -k builds/utilities/mesa/* &>> $LOG
dos2unix -k builds/utilities/CreateRenderers.sh &>> $LOG
dos2unix -k builds/utilities/README.md &>> $LOG
dos2unix -k builds/utilities/set-ldrawdir.command &>> $LOG
dos2unix -k builds/utilities/update-config-files.sh &>> $LOG
dos2unix -k builds/utilities/version.info &>> $LOG
dos2unix -k builds/linux/docker-compose/* &>> $LOG
dos2unix -k builds/linux/docker-compose/dockerfiles/* &>> $LOG
dos2unix -k builds/linux/obs/* &>> $LOG
dos2unix -k builds/linux/obs/alldeps/* &>> $LOG
dos2unix -k builds/linux/obs/alldeps/debian/* &>> $LOG
dos2unix -k builds/linux/obs/debian/* &>> $LOG
dos2unix -k builds/linux/obs/debian/source/* &>> $LOG
dos2unix -k builds/macx/* &>> $LOG
dos2unix -k mainApp/docs/* &>> $LOG
dos2unix -k mainApp/extras/* &>> $LOG
dos2unix -k mainApp/lpub3d.appdata.xml &>> $LOG
dos2unix -k gitversion.pri &>> $LOG
dos2unix -k README.md &>> $LOG

echo "$((COMMAND_COUNT += 1)) of ${COMMANDS} - Change Windows script line endings from LF to CRLF"
unix2dos -k builds/windows/* &>> $LOG
unix2dos -k builds/utilities/CreateRenderers.bat &>> $LOG
unix2dos -k builds/utilities/update-config-files.bat &>> $LOG
unix2dos -k builds/utilities/nsis-scripts/* &>> $LOG
unix2dos -k builds/utilities/nsis-scripts/Include/* &>> $LOG

echo "$((COMMAND_COUNT += 1)) of ${COMMANDS} - Update version.info..."
LP3D_UTIL_DIR=$(realpath "./builds/utilities")
FILE="$LP3D_UTIL_DIR/version.info"
if [[ -f "${FILE}" && -r "${FILE}" ]]
then
    echo "Arbitrary Update" > ${FILE}
fi

# update config files, increment version and increment commit count
echo "$((COMMAND_COUNT += 1)) of ${COMMANDS} - Call update-config-files from pre-commit to set last commit version and sha..."
chmod +x builds/utilities/hooks/pre-commit && \
./builds/utilities/hooks/pre-commit -o && \
./builds/utilities/hooks/pre-commit -f &>> $LOG && \
rm -f *.log
git add . &>> $LOG

echo "$((COMMAND_COUNT += 1)) of ${COMMANDS} - Stage and commit changes..."
cat << pbEOF >.git/COMMIT_EDITMSG
$COMMIT_MSG

pbEOF
git commit -m "$COMMIT_MSG" &>> $LOG
echo "Done!" && echo
