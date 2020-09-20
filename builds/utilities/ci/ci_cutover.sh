#!/bin/bash
# Trevor SANDY
# Last Update Sep 16, 2019
#
# Purpose:
# This script is used to 'cut-over' the development repository [lpub3d-ci] commits to production [lpub3d].
#
# Setup:
# For successful execution it must be placed at the root of the repositories, for example:
#   ./lpub3d
#   ./lpub3d-ci
#   ./ci_cutover.sh
# GitHub API, Dropbox oauth, Sourceforge rsa keys must be placed in specific folder at the repositories root:
#   ./Production_cutover/secrets/
#
# Execution Steps:
#
# Preq 1 of 3 Enable script execution [execute once] 
# $ chmod +x ci_cutover.sh && ./ci_cutover.sh
#
# Preq 2 of 3 Set 'Next' version number [execute once]
# $ sed 's/2.3.14/<next version>/g' -i ci_cutover.sh
#
# Preq 3 of 3 create and checkout a CUTOVER branch in the lpub3d-ci repository at the 
#   last production commit. [execute once]
#
# Step 1 of 3 from the CUTOVER branch add your master/dev branch commit by executing 
#   'reset to this commit' or 'cherrypick commit' [execute for each commit]
#
# Step 2 of 3 Production commits [execute for each commit except the final one for version change]
# $ env MSG="<commit> #No" OBS_CFG=yes TAG=v2.3.14 ./ci_cutover.sh
#
# Step 3 of 3 Final commit and production version change [execute once for version 
#   change [BE CAREFUL - THIS ADDS A TAG]
# $ env MSG="LPub3D v2.3.14" TAG=v2.3.14 RELEASE=yes REV=no CNT=yes OBS_CFG=yes ./ci_cutover.sh
#
# Execution sequence:
#   - copy lpub3d-ci content to lpub3d folder
#   - preserve lpub3d git database
#   - rename all files with 'lpub3d-ci' in the name to 'lpub3d'
#   - change all occurrences of 'lpub3d-ci' to 'lpub3d'
#   - update README.md Title - remove or change ' - Dev, CI, and Test'
#   - create pre-commit githook
#   - create .secrets.tar.unc file
#   - add version to config files (create new tag, run config-files...)
#   - if standard commit, delete build tab
#
# Environment variables:
#   - TAG: git tag [Default=v2.3.14] - change as needed
#   - MSG: git commit message [Default='Continuous integration cutover [build pkg]'] - change as needed
#   - FRESH: delete current lpub3d directory otherwise, only overwrite existing files [default=no]
#   - REV: increment revision [Default=yes]
#   - CNT: increment commit count [Default=yes]
#   - OBS_CFG: Set OBS config and README file updates [Default=no]
#   - RELEASE: Release build, do not delete build tag [Default=no]
#
# Command Examples:
# $ chmod +x ci_cutover.sh && ./ci_cutover.sh
# $ env MSG="LPub3D pre-release [build pkg]" TAG=v2.3.14 ./ci_cutover.sh
# $ env MSG="LPub3D version 2.3.14" RELEASE=yes REV=no OBS_CFG=yes ./ci_cutover.sh
# $ env FRESH=yes MSG="LPub3D version 2.3.14" TAG=v2.3.14 REV=no OBS_CFG=yes ./ci_cutover.sh
# $ env FRESH=yes MSG="LPub3D pre-release [build pkg]" TAG=v2.3.14 REV=no CNT=yes OBS_CFG=yes ./ci_cutover.sh
# $ env FRESH=yes MSG="Issue template and renderer logging updates" OBS_CFG=yes ./ci_cutover.sh
#
# Move to lpub3d-obs repository
# $ env GIT_NAME=lpub3d-obs MSG="Open Build Service Integration and Test" TAG=v2.3.14 ./ci_cutover.sh

cd "/mnt/c/Users/Trevor/Projects" # ~/Projects

SCRIPT_NAME=$0
SCRIPT_ARGS=$*
LOCAL_TAG=${TAG:-v2.3.14}
FRESH_BUILD=${FRESH:-no}
INC_REVISION=${REV:-yes}
INC_COUNT=${CNT:-yes}
FORCE_CONFIG=${OBS_CFG:-no}
TO_NAME=${GIT_NAME:-lpub3d}
FROM_NAME=${DEV_NAME:-lpub3d-ci}
RELEASE_COMMIT=${RELEASE:-no}
COMMIT_MSG="${MSG:-LPub3D ${TAG}}"

function options_status
{
    echo
    echo "--Command Options:"
    echo "--SCRIPT_NAME....$SCRIPT_NAME"
    echo "--FROM_REPO_NAME..$FROM_REPO_NAME"    
    echo "--TO_REPO_NAME....$TO_REPO_NAME"
    echo "--RELEASE_COMMIT..$RELEASE_COMMIT"
    [ -n "$SCRIPT_ARGS" ] && echo "--SCRIPT_ARGS.....$SCRIPT_ARGS" || true
    echo "--FRESH_BUILD.....$FRESH_BUILD"
    echo "--LOCAL_TAG.......$LOCAL_TAG"
    echo "--COMMIT_MSG......$COMMIT_MSG"
    [ -n "$INC_REVISION" ] && echo "--INCREMENT_REV...$INC_REVISION" || true
    [ -n "$FORCE_CONFIG" ] && echo "--OBS_CONFIG......$FORCE_CONFIG" || true
    echo 
}

function show_options_status
{
    COMMIT_MSG="No commit specified"
    SCRIPT_ARGS="No arguments specified - show Options and exit"
    options_status
    exit 1
}

# if [ -z "$SCRIPT_ARGS" ] ; then 
   # show_options_status 
# fi
    
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
f="${CWD}/$ME"
ext=".log"
if [[ -e "$f$ext" ]] ; then
    i=1
    f="${f%.*}";
    while [[ -e "${f}_${i}${ext}" ]]; do
      let i++
    done
    f="${f}_${i}${ext}"
    else
    f="${f}${ext}"
fi
# output log file
LOG="$f"
exec > >(tee -a ${LOG} )
exec 2> >(tee -a ${LOG} >&2)

# Show options
options_status

# Remove current lpub3d folder and clone fresh instance if requested
[ "$FRESH_BUILD" != "no" ] && [ -d "$TO_REPO_NAME" ] && \
echo && echo "--Remove old $TO_REPO_NAME instance..." && rm -rf "$TO_REPO_NAME" || true

# Clone new instance of lpub3d if old instance does not exist or was removed
if [ ! -d "$TO_REPO_NAME" ]; then
    echo && echo "1-Creating new $TO_REPO_NAME instance..."
    git clone https://github.com/trevorsandy/${TO_REPO_NAME}.git
else
    echo && echo "1-Updating existing $TO_REPO_NAME instance..."
fi

cd $HOME_DIR/$FROM_REPO_NAME
if [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3d" ]]; then git checkout CUTOVER;
elif [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then git checkout NEXT_OUT; 
fi

cd $HOME_DIR/$TO_REPO_NAME
if [ "$TO_REPO_NAME" = "lpub3dnext" ];then git checkout CUTOVER_IN; fi

echo "2-Remove current $TO_REPO_NAME content except .git folder..."
find . -not -path "./.git/*" -type f -exec rm -rf {} +

echo "3-Copy $FROM_NAME content to $TO_NAME except .git folder..." && cd ../$FROM_NAME
find . -not -name '*.log*' \
       -not -name '*.user' \
       -not -name '*Makefile*' \
       -not -name '*Makefile*' \
       -not -name '*_resource.rc*' \
       -not -name '*_object_script.rc' \
       -not -name '*_resource.rc*' \
       -not -name '*AppVersion.nsh' \
       -not -path "./.git/*" \
       -not -path './builds/windows/release' \
       -not -path './ldvlib/LDVQt/*bit_release' \
       -not -path './ldvlib/LDVQt/include/*' \
       -not -path './ldvlib/WPngImage/*bit_release' \
       -not -path './lclib/*bit_release' \
       -not -path './mainApp/*bit_release' \
       -not -path './quazip/*bit_release' \
       -not -path './ldrawini/*bit_release' \
       -type f -exec cp -f --parents -t ../$TO_NAME {} +
cp -f ./.gitignore ../$TO_NAME

echo "4-Rename all files with '$FROM_NAME' in the name to '$TO_NAME'..." && cd ../$TO_NAME
for file in $(find . -type f -name "*${FROM_NAME}*" \
              -not -path "./.git*" \
              -not -path "./mainApp/qdarkstyle*" \
              -not -path "./lclib*" \
              -not -path "./ldvlib*" \
              -not -path "./ldrawini*" \
              -not -path "./qslog*" \
              -not -path "./quazip*" \
              -not -path "./qsimpleupdater*" \
              )
do
    if [ "$TO_REPO_NAME" = "lpub3d" ]; then newFile=$(echo $file | sed s/-ci//g);
    elif [ "$TO_REPO_NAME" = "lpub3d-obs" ]; then newFile=$(echo $file | sed s/-ci/-obs/g);
    elif [ "$TO_REPO_NAME" = "lpub3dnext" ]; then newFile=$(echo $file | sed s/-ci/next/g);
    elif [ "$TO_REPO_NAME" = "lpub3d-ci" ]; then newFile=$(echo $file | sed s/lpub3dnext/lpub3d-ci/g);
    fi
    mv -f $file $newFile
    [ -f $newFile ] && echo " -file changed: $newFile."
done
if [ "$TO_REPO_NAME" = "lpub3dnext" ]; then 
  projFile="LPub3D.pro"
  newProjFile="LPub3DNext.pro"
  mv -f $projFile $newProjFile
  [ -f $newProjFile ] && echo " -file changed: $newProjFile."  
elif [ "$FROM_REPO_NAME" = "lpub3dnext" ]; then
  projFile="LPub3DNext.pro"
  newProjFile="LPub3D.pro"
  mv -f $projFile $newProjFile
  [ -f $newProjFile ] && echo " -file changed: $newProjFile."  
fi

echo "5-Change occurrences of '$FROM_NAME' to '$TO_NAME' in files..."
for file in $(find . -type f \
              -not -path "./.git/*" \
              -not -path "./mainApp/images*" \
              -not -path "./mainApp/qdarkstyle*" \
              -not -path "./lclib*" \
              -not -path "./ldvlib*" \
              -not -path "./ldrawini*" \
              -not -path "./qslog*" \
              -not -path "./quazip*" \
              -not -path "./qsimpleupdater*" \
              -not -path "./builds/utilities/ci/ci_cutover.sh*" \
              )
do
    cat $file | grep -qE "$FROM_NAME" \
    && sed "s/${FROM_NAME}/${TO_NAME}/g" -i $file \
    && echo " -file updated: $file" || true
done

echo "6-Update sfdeploy.sh"
file=builds\utilities\ci\sfdeploy.sh
if [ "$TO_NAME" = "lpub3d" ]; then sed s/'--dry-run '//g -i $file; echo " -file $file updated.";
else echo " -ERROR - file $file NOT updated.";
fi

echo "7-Update README.md Title"
file=README.md
if [ "$TO_NAME" = "lpub3d" ]; then sed s/' - Dev, CI, and Test'//g -i $file; echo " -file $file updated.";
elif [ "$TO_NAME" = "lpub3d-obs" ]; then sed s/' - Dev, CI, and Test'/' - Open Build Service Integration and Test'/g -i $file; echo " -file $file updated.";
else echo " -ERROR - file $file NOT updated.";
fi

echo "8-Create pre-commit githook..."
cat << pbEOF >.git/hooks/pre-commit
#!/bin/sh
./builds/utilities/hooks/pre-commit   # location of pre-commit script in source repository
pbEOF

echo "9-Create .secrets.tar"
secretsDir=$(cd ../ && echo $PWD)/Production_cutover/secrets
secureDir=builds/utilities/ci/secure
file=builds/utilities/ci/travis/before_install
cp -f $secretsDir/.dropbox_oauth .
cp -f $secretsDir/.sfdeploy_travis_rsa .
tar cvf .secrets.tar .dropbox_oauth .sfdeploy_travis_rsa
[ -f .secrets.tar ] && travis encrypt-file .secrets.tar || echo " -.secrets.tar not found."
[ -f .secrets.tar.enc ] && mv .secrets.tar.enc $secureDir/.secrets.tar.enc \
&& rm -f .secrets.tar .dropbox_oauth .sfdeploy_travis_rsa \
&& echo " -.secrets.tar.enc moved to secure dir and secrets files removed." \
|| echo " -.secrets.tar.enc not found."
sed s/'cacf73a1954d'/'ad0a5b26bc84'/g -i $file \
&& echo " -file $file updated with secure variable key." \
|| echo " -ERROR - file $file NOT updated."

echo "10-Change *.sh line endings from CRLF to LF"
for file in $(find . -type f -name *.sh)
do
    dos2unix -k $file
done
echo "11-Change other line endings from CRLF to LF"
dos2unix -k builds/utilities/hooks/*
dos2unix -k builds/utilities/create-dmg
dos2unix -k builds/utilities/dmg-utils/*
dos2unix -k builds/utilities/ci/travis/*
dos2unix -k builds/utilities/mesa/*
dos2unix -k builds/utilities/set-ldrawdir.command
dos2unix -k builds/linux/docker-compose/*
dos2unix -k builds/linux/docker-compose/dockerfiles/*
dos2unix -k builds/linux/obs/*
dos2unix -k builds/linux/obs/alldeps/*
dos2unix -k builds/linux/obs/alldeps/debian/*
dos2unix -k builds/linux/obs/debian/*
dos2unix -k builds/linux/obs/debian/source/*
dos2unix -k builds/macx/*

echo "12-Change Windows script line endings from LF to CRLF"
unix2dos -k builds/windows/*
unix2dos -k builds/utilities/CreateRenderers.bat
unix2dos -k builds/utilities/update-config-files.bat
unix2dos -k builds/utilities/nsis-scripts/*
unix2dos -k builds/utilities/nsis-scripts/Include/*

echo "13-Patch update-config-files.sh"
file=builds/utilities/update-config-files.sh
sed -e s/'.dsc   - add'/'.dsc      - add'/g \
    -e s/'.spec  - add'/'.spec     - add'/g -i $file \
&& echo " -file $file updated." \
|| echo " -ERROR - file $file NOT updated."

echo "14-update github api_key for Travis CI"
source $(cd ../ && echo $PWD)/Production_cutover/secrets/.github_api_keys
file=.travis.yml
sed "s,^        secure: ${GITHUB_DEVL_SECURE_API_KEY},        secure: ${GITHUB_PROD_SECURE_API_KEY}," -i $file \
&& echo " -file $file updated." \
|| echo " -ERROR - file $file NOT updated."

echo "15-Add new files..."
git add .
git reset HEAD 'mainApp/docs/README.txt'

echo "16-Create local tag in $TO_NAME repository"
if GIT_DIR=./.git git rev-parse $LOCAL_TAG >/dev/null 2>&1; then git tag --delete $LOCAL_TAG; fi
git tag -a $LOCAL_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
git_tag="$(git tag -l -n $LOCAL_TAG)" && \
[ -n "$git_tag" ] && echo " -git tag $git_tag created."

echo "17-Stage and commit changed files..."
cat << pbEOF >.git/COMMIT_EDITMSG
$COMMIT_MSG

pbEOF
env force=$FORCE_CONFIG inc_rev=$INC_REVISION inc_cnt=$INC_COUNT git commit -m "$COMMIT_MSG"

if [ "$RELEASE_COMMIT" = "no" ]; then 
   echo "18-Delete local tag"
   git tag --delete $LOCAL_TAG
   rm -f update-config-files.sh.log
else
   echo "18-Create local tag in $FROM_NAME repository" 
   cd ../$FROM_NAME
   if GIT_DIR=./.git git rev-parse $LOCAL_TAG >/dev/null 2>&1; then git tag --delete $LOCAL_TAG; fi
   git tag -a $LOCAL_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
   git_tag="$(git tag -l -n $LOCAL_TAG)" && \
   [ -n "$git_tag" ] && echo " -git tag $git_tag created."
   cd ../
fi

echo "Finished - COMMIT_MSG...$COMMIT_MSG" && echo

