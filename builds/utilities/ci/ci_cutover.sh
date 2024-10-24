#!/bin/bash
# Trevor SANDY
# Last Update: October 19, 2024
# Copyright (C) 2024 by Trevor SANDY

function ShowHelp() {
    echo
    echo $0
    echo
    echo "Written by Trevor SANDY"
    echo
    echo "Purpose:"
    echo "This script is used to 'cutover' development [lpub3dnext] or maintenance [lpub3d] repository commits, one at a time, to production."
    echo "However, commits can be moved between any of the receiving or sourced repositories."
    echo
    echo "Note: Use the accompanying script: 'next_cutover.sh' to automate the 'cutover' of a range of commits."
    echo
    echo "Setup:"
    echo "For successful execution it must be placed at the root of the repositories, for example:"
    echo "  ./lpub3d"
    echo "  ./lpub3d"
    echo "  $0"
    echo
    echo "Execution Steps:"
    echo
    echo "Note: Use the accompanying script: 'next_cutover.sh' to automate the 'cutover' of a range of commits."
    echo
    echo "Preq 1 of 3 Enable script execution [execute once]"
    echo "  \$ chmod +x $0 && $0"
    echo
    echo "Preq 2 of 3 Set 'Next' version number [execute once]"
    echo "  \$ sed 's/2.4.8/<next version>/g' -i $0"
    echo
    echo "Preq 3 of 3 create and checkout a CUTOVER branch in the lpub3d repository at the"
    echo "  last production commit. [execute once]"
    echo
    echo "Step 1 of 3 from the CUTOVER branch add your master/dev branch commit by executing"
    echo "  'reset to this commit' or 'cherrypick commit' [execute for each commit]"
    echo
    echo "Step 2 of 3 Production commits [execute for each commit except the final one for version change]"
    echo "  \$ env MSG='<commit> echoNo' CFG=yes TAG=v2.4.8 $0"
    echo
    echo "Step 3 of 3 Final commit and production version change [execute once for version"
    echo "  change [BE CAREFUL - THIS ADDS A TAG]"
    echo "  \$ env MSG='LPub3D v2.4.8' TAG=v2.4.8 REL=1  CFG=yes $0"
    echo
    echo "Step 4 of 5 Copy README.txt and RELEASE_NOTES.html from 'lpub3d' back to 'lpub3d'"
    echo "  \$ cp -f lpub3d/mainApp/docs/README.txt lpub3d/mainApp/docs/"
    echo "  \$ cp -f lpub3d/mainApp/docs/RELEASE_NOTES.html lpub3d/mainApp/docs/"
    echo
    echo "Execution sequence:"
    echo "  - copy lpub3d content to lpub3d folder"
    echo "  - preserve lpub3d git database"
    echo "  - rename all files with 'lpub3d' in the name to 'lpub3d'"
    echo "  - change all occurrences of 'lpub3d' to 'lpub3d'"
    echo "  - update README.md Title - remove or change ' - Dev, CI, and Test'"
    echo "  - create pre-commit githook"
    echo "  - create .secrets.tar.unc file"
    echo "  - add version to config files (create new tag, run config-files...)"
    echo "  - if standard commit, delete build tab"
    echo
    echo "Environment variables:"
    echo "  - REPO_BASE: Repository base URL [Default=https://github.com/trevorsandy]"
    echo "  - FROM_REPO: Development repository [Default=lpub3d]"
    echo "  - TO_REPO: Production or maintenance repository [Default=lpub3d]"
    echo "  - MSG: Git commit message [Default='LPub3D v2.4.8'] - change as needed"
    echo "  - TAG: Git tag [Default=null] - change as needed"
    echo "  - NEW_TAG: Next Git tag [Default=null] - change as needed"
    echo "  - AUTO: Do not prompt to continue after pausing at options status [Default=null]"
    echo "  - CFG: Set OBS config and README file updates [Default=no]"
    echo "  - REL: Release build, do not delete build tag [Default=no]"
    echo "  - REV: Increment revision [Default=yes]"
    echo "  - CNT: Increment commit count [Default=yes]"
    echo "  - NOSTAT: Do not show the options_status [Default=null]"
    echo "  - MIN_RN_LINE_DEL: Start line to delete when truncating RELEASE_NOTES [Default=null]"
    echo "  - MAX_RN_LINE_DEL: Stop Line to delete when truncating RELEASE_NOTES [Default=null]"
    echo "  - FRESH: Clone a new instance of TO_REPO otherwise, only overwrite existing files [Default=no]"
    echo
    echo "Command Examples:"
    echo "  \$ chmod +x $0 && $0"
    echo "  \$ env MSG='LPub3D pre-release [build pkg]' TAG=v2.4.8 $0"
    echo "  \$ env MSG='LPub3D version 2.4.8' REL=1 CFG=yes $0"
    echo "  \$ env FRESH=yes MSG='LPub3D version 2.4.8' TAG=v2.4.8 CFG=yes $0"
    echo "  \$ env FRESH=yes MSG='LPub3D pre-release [build pkg]' TAG=v2.4.8 CFG=yes $0"
    echo "  \$ env FRESH=yes MSG='Issue template and renderer logging updates' CFG=yes $0"
    echo
    echo "Move from lpub3d to lpub3d-obs repository"
    echo "  \$ env TO_REPO=lpub3d-obs MSG='Open Build Service Integration and Test' TAG=v2.4.8 $0"
    echo
    echo "Move from lpub3d to lpub3d repository"
    echo "Step 1 of 2 Production commits [execute for each commit except the final one for version change]"
    echo "  \$ env TO_REPO=lpub3dnext MSG='<commit hash> <commit message>' TAG=v2.4.8 $0"
    echo
    echo "Step 2 of 2 Final production commit version change [execute once]"
    echo "  \$ env TO_REPO=lpub3dnext MSG='<commit hash> <commit message>' TAG=v2.4.8 REL=1 \\"
    echo "        MAX_RN_LINE_DEL=<number> MIN_RN_LINE_DEL=<number>  CFG=yes $0"
    echo
    echo "Step 1 of 2 Maintenance commits [Change lpub3dnext branch to CUTOVER_CI and execute for each"
    echo "commit except the final one for version change]"
    echo "  \$ env TO_REPO=lpub3dnext MSG='<commit hash> <commit message>' TAG=v2.4.8 $0"
    echo
    echo "Step 2 of 2 Final maintenance commit version change [execute once]"
    echo "  \$ env TO_REPO=lpub3dnext MSG='<commit hash> <commit message>' TAG=v2.4.8 REL=1 CFG=yes $0"
    echo
    echo "Move from lpub3dnext to lpub3d repository"
    echo "Step 1 of 1 Maintenance commits [Change lpub3d branch to NEXT_IN and execute for each"
    echo "commit except the final one for version change]"
    echo "  \$ env TO_REPO=lpub3d FROM_REPO=lpub3dnext MSG='<commit hash> <commit message>' TAG=v2.4.8 CFG=yes $0"
    echo
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -?|-h|--help) ShowHelp; exit 0 ;;
        Deb|Pkg|Rpm) continue ;;
        *) echo "Unknown parameter passed: '$1'. Use -? to show help."; exit 1 ;;
    esac
    shift
done

SCRIPT_NAME=$0
SCRIPT_ARGS=$*
HOME_DIR=$PWD

AUTO_MODE=${AUTO:-}
NO_STATUS=${NOSTAT:-}
LOCAL_TAG=${TAG:-}
NEW_VER_TAG=${NEW_TAG:-}
FRESH_BUILD=${FRESH:-}
INC_REVISION=${REV:-yes}
INC_COUNT=${CNT:-yes}
FORCE_CONFIG=${CFG:-no}
TO_REPO_NAME=${TO_REPO:-lpub3d}
FROM_REPO_NAME=${FROM_REPO:-lpub3d}
REPO_BASE_URL=${REPO_BASE:-https://github.com/trevorsandy}
RELEASE_COMMIT=${REL:-}
MIN_RN_LN_DEL=${MIN_RN_LINE_DEL:-}
MAX_RN_LN_DEL=${MAX_RN_LINE_DEL:-}
COMMIT_MSG=${MSG:-LPub3D ${TAG}}
NEXT_VER_TAG=${NEXT_VER_TAG:-}

COMMAND_COUNT=0

# Set increment revision except for final release commit
[ -n "$RELEASE_COMMIT" ] && INC_REVISION=no || :

# Set LOCAL_TAG if not set from command
[ -z "$LOCAL_TAG" ] && LOCAL_TAG="$(cd $HOME_DIR/$FROM_REPO_NAME ; git describe --abbrev=0)"

# Set NEW_VER_TAG if new version tag
[[ -n "$RELEASE_COMMIT" && "${NEW_VER_TAG}" == "${LOCAL_TAG}" ]] && NEXT_VER_TAG=yes || :

function options_status
{
    echo
    echo "--Cutover Command Options:"
    echo "--SCRIPT_NAME....$SCRIPT_NAME"
    echo "--FROM_REPO_NAME..$FROM_REPO_NAME"
    echo "--TO_REPO_NAME....$TO_REPO_NAME"
    echo "--COMMIT_MSG......$COMMIT_MSG"
    echo "--REPO_PATH.......$HOME_DIR"
    echo "--LOCAL_TAG.......$LOCAL_TAG"
    [ -n "$NEW_VER_TAG" ] && echo "--NEW_VER_TAG.....$NEW_VER_TAG" || echo "--NEW_VER_TAG.....$LOCAL_TAG"
    [ "$NEXT_VER_TAG" = "yes" ] && echo "--NEXT_VER_TAG....YES" || echo "--NEXT_VER_TAG....NO"
    [ "$INC_REVISION" = "yes" ] && echo "--INCREMENT_REV...YES" || echo "--INCREMENT_REV...NO"
    [ "$INC_COUNT" = "yes" ] && echo "--INCREMENT_CNT...YES" || echo "--INCREMENT_CNT...NO"
    [ "$FORCE_CONFIG" = "yes" ] && echo "--OBS_CONFIG......YES" || echo "--OBS_CONFIG......NO"
    [ -n "$RELEASE_COMMIT" ] && echo "--RELEASE_COMMIT..YES" || echo "--RELEASE_COMMIT..NO"
    [ -n "$FRESH_BUILD" ] && echo "--FRESH_BUILD.....YES" || echo "--FRESH_BUILD.....NO"
    [ -n "$SCRIPT_ARGS" ] && echo "--SCRIPT_ARGS.....$SCRIPT_ARGS" || true
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

echo
echo "=============================================================================="

# Show options
[ -z "$NO_STATUS" ] && options_status || echo

# Confirmation
if [ -z "AUTO_MODE" ]
then
    sleep 1s && read -p "  Are you sure (y/n)? " -n 1 -r
    echo    # (optional) move to a new line
    if [[ ! $REPLY =~ ^[Yy]$ ]]
    then
    [[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1 # handle exits from shell or function but don't exit interactive shell
    fi
fi

# Remove current lpub3d folder and clone fresh instance if requested
if [[ -n "$FRESH_BUILD" && -d "$TO_REPO_NAME" ]]; then
    echo "--Attempting to remove old $TO_REPO_NAME instance..."
    cd $HOME_DIR/$TO_REPO_NAME
    if [ -d ".git" ]
    then
        if [ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]; then
            git checkout master
        fi
        if [ -n "$(git status -s)" ]; then
            echo "--ERROR - Uncommitted changes detected - exiting..."
            cd $HOME_DIR
            exit 1
        fi
    fi
    cd $HOME_DIR
    rm -rf "$TO_REPO_NAME"
    if [ -d "$TO_REPO_NAME" ]; then
        echo "--ERROR - Could not remove $TO_REPO_NAME - exiting..."
        exit 1
    else
        echo "--Removed $TO_REPO_NAME."
    fi
fi

# Clone new instance of lpub3d if old instance does not exist or was removed
if [ ! -d "$TO_REPO_NAME" ]; then
    echo "$((COMMAND_COUNT += 1))-Creating new $TO_REPO_NAME instance..."
    git clone ${REPO_BASE_URL}/${TO_REPO_NAME}.git
else
    echo "$((COMMAND_COUNT += 1))-Updating existing $TO_REPO_NAME instance..."
fi

# Verify we are in the right source repository branch
cd $HOME_DIR/$FROM_REPO_NAME
if [ -n "$(git status -s)" ]; then
    echo "--INFO - Stashing uncommitted $FROM_REPO_NAME changes..."
    git stash
fi
if [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d" ]]; then
    START_BRANCH=CUTOVER_CI;
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then
    START_BRANCH=CUTOVER_CI;
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then
    START_BRANCH=CUTOVER_NEXT;
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then
    START_BRANCH=CUTOVER_NEXT;
else
    START_BRANCH=master;       # cutover production to development or maintenance
fi
if [ -n "$(git show-ref refs/heads/$START_BRANCH)" ]
then # start branch exist
    if [ "$(git rev-parse --abbrev-ref HEAD)" != "$START_BRANCH" ]
    then # start branch not checked out so do so
        git checkout $START_BRANCH
    fi
else # start branch does not exist so exit
    echo "-- VERY WIERD! Branch '$START_BRANCH' does not exist - exiting..."
    exit 1
fi

# Prepare destination repository branch
cd $HOME_DIR/$TO_REPO_NAME

if [ -n "$(git status -s)" ]; then
    echo "--INFO - Stashing uncommitted $TO_REPO_NAME changes..."
    git stash
fi
if [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]];then
    START_BRANCH=master
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then
    START_BRANCH=CUTOVER_CI
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]];then
    START_BRANCH=CUTOVER_NEXT
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d" ]]; then
    START_BRANCH=CUTOVER_CI
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]];then
    START_BRANCH=CUTOVER_NEXT
fi
if [ -n "$(git show-ref refs/heads/$START_BRANCH)" ]
then # start branch exist
    if [ "$(git rev-parse --abbrev-ref HEAD)" != "$START_BRANCH" ]
    then # start branch not checked out so do so
        git checkout $START_BRANCH
    fi
else # start branch does not exist so create it and checkout
    git checkout -b $START_BRANCH
fi
if [ "$(git rev-parse --abbrev-ref HEAD)" = "$START_BRANCH" ]
then # Test that all is ok
    echo "  -branch $START_BRANCH checked out."
else
    echo "  -MY GOODNESS! Branch $START_BRANCH checkout failed - exiting."
    exit 1
fi

echo "$((COMMAND_COUNT += 1))-Remove current $TO_REPO_NAME content except .git folder and .user file..."
find . -not -name '*.user' -not -path "./.git/*" -type f -exec rm -rf {} +

echo "$((COMMAND_COUNT += 1))-Copy $FROM_REPO_NAME content to $TO_REPO_NAME except .git folder and .user file..." && cd ../$FROM_REPO_NAME
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
       -not -path './lclib/*bit_release' \
       -not -path './ldrawini/*bit_release' \
       -not -path './ldvlib/LDVQt/*bit_release' \
       -not -path './ldvlib/LDVQt/include/*' \
       -not -path './ldvlib/WPngImage/*bit_release' \
       -not -path './mainApp/*bit_release' \
       -not -path './quazip/*bit_release' \
       -not -path './waitingspinner/*bit_release' \
       -type f -exec cp -f --parents -t ../$TO_REPO_NAME {} +
cp -f ./.gitignore ../$TO_REPO_NAME

cd ../$TO_REPO_NAME

devops_build_yml=devops_ci_build.yml
prod_build_yml=prod_ci_build.yml
echo -n "$((COMMAND_COUNT += 1))-Rename actions file from '$devops_build_yml' to '$prod_build_yml'..."
(cd .github/workflows && mv -f "$devops_build_yml" "$prod_build_yml") && echo OK

echo "$((COMMAND_COUNT += 1))-Change occurrences of '$devops_build_yml' to '$prod_build_yml' in files..."
for file in \
README.md \
builds/utilities/ci/github/build-deploy.sh \
builds/utilities/ci/github/linux-build.sh \
builds/utilities/ci/github/linux-multiarch-build.sh \
builds/utilities/ci/github/macos-build.sh \
builds/utilities/ci/github/windows-build.bat \
mainApp/otherfiles.pri
do
    cat $file | grep -qE "$devops_build_yml" \
    && sed "s/$devops_build_yml/$prod_build_yml/g" -i $file \
    && echo "  -file updated: $file" || true
done

echo "$((COMMAND_COUNT += 1))-Rename all files with '$FROM_REPO_NAME' in the name to '$TO_REPO_NAME'..."
for file in $(find . -type f -name "*${FROM_REPO_NAME}*" \
              -not -path "./.git*" \
              -not -path "./lclib*" \
              -not -path "./ldrawini*" \
              -not -path "./ldvlib*" \
              -not -path "./qsimpleupdater*" \
              -not -path "./qslog*" \
              -not -path "./quazip*" \
              -not -path "./waitingspinner*" \
              -not -path "./mainApp/extras/LPub3D_Npp_UDL.xml" \
              -not -path "./builds/utilities/ci/next_cutover.sh" \
              )
do
    if   [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d" ]]; then newFile=$(echo $file | sed s/-ci//g);
    elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then newFile=$(echo $file | sed s/-ci/next/g);
    elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then newFile=$(echo $file | sed s/next//g);
    elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then newFile=$(echo $file | sed s/next/-ci/g);
    elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dNext" ]]; then newFile=$(echo $file | sed s/lpub3d/lpub3dnext/g);
    elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d" ]]; then newFile=$(echo $file | sed s/lpub3d/lpub3d/g);
    fi
    mv -f "$file" "$newFile"
    [ -f "$newFile" ] && echo "  -file renamed: $newFile."
done
# rename project file with '$FROM_REPO_NAME' in the name to '$TO_REPO_NAME'..."
if [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then
    projFileName="LPub3D.pro"
    newProjFileName="LPub3DNext.pro"
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then
    projFileName="LPub3DNext.pro"
    newProjFileName="LPub3D.pro"
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then
    projFileName="LPub3D.pro"
    newProjFileName="LPub3DNext.pro"
fi
if [[ -f "$projFileName" && -n "$newProjFileName" && ($projFileName != $newProjFileName) ]]; then
    mv -f "$projFileName" "$newProjFileName"
    [ -f "$newProjFileName" ] && echo "  -file changed: $newProjFileName." || echo "  -ERROR - $newProjFileName was not renamed."
fi

echo "$((COMMAND_COUNT += 1))-Change occurrences of '$FROM_REPO_NAME' to '$TO_REPO_NAME' in files..."
for file in $(find . -type f \
              -not -path "./.git/*" \
              -not -path "./mainApp/images*" \
              -not -path "./mainApp/resources/*" \
              -not -path "./mainApp/*.h" \
              -not -path "./mainApp/*.cpp" \
              -not -path "./mainApp/*.ui" \
              -not -path "./lclib*" \
              -not -path "./ldvlib*" \
              -not -path "./ldrawini*" \
              -not -path "./qslog*" \
              -not -path "./quazip*" \
              -not -path "./qsimpleupdater*" \
              -not -path "./waitingspinner*" \
              -not -path "./builds/utilities/icons/*" \
              -not -path "./builds/utilities/ci/build_checks.sh" \
              -not -path "./builds/utilities/ci/travis/releases" \
              -not -path "./builds/utilities/ci/$0" \
              -not -path "./builds/utilities/ci/next_cutover.sh" \
              -not -path './.github/workflows/$prod_build_yml' \
              -not -path "./gitversion.pri" \
              -not -path "./snapcraft.yaml" \
              -not -path "./appveyor.yml" \
              -not -path "./travis.yml" \
              )
do
    cat $file | grep -qE "$FROM_REPO_NAME" \
    && sed "s/${FROM_REPO_NAME}/${TO_REPO_NAME}/g" -i $file \
    && echo "  -file updated: $file" || true
done
backslash='\\'
prerelease='true'
# This is corrupting my versioning scheme so i'll disable for now
#if [ "$TO_REPO_NAME" = "lpub3d" ]; then prerelease='false'; fi
file=appveyor.yml
sed -e "s,clone_folder: c:${backslash}projects${backslash}$FROM_REPO_NAME,clone_folder: c:${backslash}projects${backslash}$TO_REPO_NAME," \
    -e "s,^   repository: trevorsandy/$FROM_REPO_NAME,   repository: trevorsandy/$TO_REPO_NAME," \
    -e "s,^   prerelease: true,   prerelease: ${prerelease}," -i $file \
&& echo "  -file $file updated." || echo "  -ERROR - file $file NOT updated."

echo "$((COMMAND_COUNT += 1))-Update 'dry-run' flag in sfdeploy.sh"
file=builds/utilities/ci/sfdeploy.sh
if [ "$TO_REPO_NAME" = "lpub3d" ]; then sed s/'--dry-run '//g -i $file; echo "  -file $file updated.";
else sed s/'rsync --recursive'/'rsync --dry-run --recursive'/g -i $file; echo "  -NOTICE - $TO_REPO_NAME not production, 'dry-run' preserved in $file.";
fi

echo "$((COMMAND_COUNT += 1))-Update README.md Title"
file=README.md
if   [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d" ]]; then sed s/' - Dev, CI, and Test'//g -i $file; echo "  -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then s/' - Dev, CI, and Test'/' - Next Development'/g -i $file; echo "  -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then sed s/' - Next Development'//g -i $file; echo "  -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then s/' - Next Development'/' - Dev, CI, and Test'/g -i $file; echo "  -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dNext" ]]; then s/'# LPub3D'/'# LPub3D - Next Development'/g -i $file; echo "  -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d" ]]; then s/'# LPub3D'/'# LPub3D - Dev, CI, and Test'/g -i $file; echo "  -file $file updated.";
else echo "  -ERROR - file $file NOT updated.";
fi
SED_OPTIONS=("-re" "s,v(([0-9]+\.)*[0-9]+)\.svg\?label=revision,${LOCAL_TAG}\.svg\?label=revision," \
             "-re" "s,v(([0-9]+\.)*[0-9]+)\&logo=data,${LOCAL_TAG}\&logo=data," \
             "-re" "s,continuous \"Revisions since v(([0-9]+\.)*[0-9]+)\",continuous \"Revisions since ${LOCAL_TAG}\"," \
             "-e" "s,projects\/1 \"Last edited.*\",projects/1 \"Last edited $(date +%d-%m-%Y)\"," \
             "-i")
sed "${SED_OPTIONS[@]}" $file && echo "  -file $file updated." || echo "  -ERROR - file $file NOT updated."

if [[ -n "$RELEASE_COMMIT" && -n "$MAX_RN_LN_DEL" && -n "$MIN_RN_LN_DEL" ]]; then
  echo "$((COMMAND_COUNT += 1))-Truncate RELEASE_NOTES.html, remove lines ${MIN_RN_LN_DEL} to ${MAX_RN_LN_DEL}."
  file=mainApp/docs/RELEASE_NOTES.html
  sed "${MIN_RN_LN_DEL},${MAX_RN_LN_DEL}d" -i $file \
  && echo "  -file $file updated." || echo "  -ERROR - file $file NOT updated."
  unset MIN_RN_LN_DEL
  unset MAX_RN_LN_DEL
fi

echo "$((COMMAND_COUNT += 1))-Create pre-commit githook..."
cat << pbEOF >.git/hooks/pre-commit
#!/bin/sh
./builds/utilities/hooks/pre-commit   # location of pre-commit script in source repository
pbEOF

echo "$((COMMAND_COUNT += 1))-Replace leading spaces with tabs for LCLib files"
counter=0
for file in $(find ./lclib -type f -name '*.h' -o -name '*.cpp')
do
    echo "Converting $file..." &>> $LOG && \
    data=$(unexpand --first-only -t 4 $file) && \
    rm "$file" && \
    echo "$data" > "$file" && \
    counter=$((counter + 1)) || true
done
echo "  -lclib files updated: $counter"

echo "$((COMMAND_COUNT += 1))-Replace leading spaces with tabs for LDVQt files"
counter=0
for file in $(find ./ldvlib/LDVQt -type f -name '*.h' -o -name '*.cpp' \
              -not -path "./ldvlib/LDVQt/include*" \
              -not -path "./ldvlib/LDVQt/resources*" \
              )
do
    echo "Converting $file..." &>> $LOG && \
    data=$(unexpand --first-only -t 4 $file) && \
    rm "$file" && \
    echo "$data" > "$file" && \
    counter=$((counter + 1)) || true
done
echo "  -ldvlib files updated: $counter"

echo "$((COMMAND_COUNT += 1))-Change *.sh line endings from CRLF to LF"
for file in $(find . -type f -name *.sh)
do
    dos2unix -k $file &>> $LOG
done

echo "$((COMMAND_COUNT += 1))-Change other line endings from CRLF to LF"
dos2unix -k builds/utilities/ci/github/* &>> $LOG
dos2unix -k builds/utilities/ci/secure/* &>> $LOG
dos2unix -k builds/utilities/ci/travis/* &>> $LOG
dos2unix -k builds/utilities/ci/$0 &>> $LOG
dos2unix -k builds/utilities/ci/next_cutover.sh &>> $LOG
dos2unix -k builds/utilities/ci/sfdeploy.sh &>> $LOG
dos2unix -k builds/utilities/dmg-utils/* &>> $LOG
dos2unix -k builds/utilities/dmg-utils/support/* &>> $LOG
dos2unix -k builds/utilities/hooks/* &>> $LOG
dos2unix -k builds/utilities/json/* &>> $LOG
dos2unix -k builds/utilities/mesa/* &>> $LOG
dos2unix -k builds/utilities/CreateRenderers.sh &>> $LOG
dos2unix -k builds/utilities/README.md &>> $LOG
dos2unix -k builds/utilities/RunCreate.sh &>> $LOG
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
dos2unix -k gitversion.pri &>> $LOG

echo "$((COMMAND_COUNT += 1))-Change Windows script line endings from LF to CRLF"
unix2dos -k builds/windows/* &>> $LOG
unix2dos -k builds/utilities/nsis-scripts/* &>> $LOG
unix2dos -k builds/utilities/nsis-scripts/Include/* &>> $LOG
unix2dos -k builds/utilities/CreateRenderers.bat &>> $LOG
unix2dos -k builds/utilities/update-config-files.bat &>> $LOG
unix2dos -k builds/utilities/win_tee.cmd &>> $LOG

echo "$((COMMAND_COUNT += 1))-Add new files..."
rm -f *.log
git add . &>> $LOG
#git reset HEAD 'mainApp/docs/README.txt'

# Create tag here to enable commit files configuration for new version tab
if [ -n "${NEXT_VER_TAG}" ]; then
  echo "$((COMMAND_COUNT += 1))-Create local tag in $TO_REPO_NAME repository"
  if GIT_DIR=./.git git rev-parse $LOCAL_TAG >/dev/null 2>&1; then git tag --delete $LOCAL_TAG; fi
  git tag -a $LOCAL_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
  git_tag="$(git tag -l -n $LOCAL_TAG)" && \
  [ -n "$git_tag" ] && echo "  -git tag $git_tag created."
fi

echo "$((COMMAND_COUNT += 1))-Stage and commit changed files..."
cat << pbEOF >.git/COMMIT_EDITMSG
$COMMIT_MSG

pbEOF
chmod a+x builds/utilities/hooks/pre-commit
env force_all=$FORCE_CONFIG inc_rev=$INC_REVISION inc_cnt=$INC_COUNT git commit -m "$COMMIT_MSG"
find . -name '*.log*' -type f -exec rm -f *.log {} +

if [ -n "$RELEASE_COMMIT" ]; then
   # Delete and recreate new version tag to place tag on last commit
   echo "$((COMMAND_COUNT += 1))-Recreate local tag $LOCAL_TAG in $TO_REPO_NAME repository"
   if GIT_DIR=./.git git rev-parse $LOCAL_TAG >/dev/null 2>&1; then git tag --delete $LOCAL_TAG; fi
   git tag -a $LOCAL_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
   git_tag="$(git tag -l -n $LOCAL_TAG)" && \
   [ -n "$git_tag" ] && echo "  -git tag $git_tag recreated."
elif [ -n "${NEXT_VER_TAG}" ]; then
   # Delete version tag after commit as we are not releasing at this commit
   echo "$((COMMAND_COUNT += 1))-Delete local tag $LOCAL_TAG in $TO_REPO_NAME repository"
   git tag --delete $LOCAL_TAG
fi

echo "$((COMMAND_COUNT += 1))-Restore repository checked out state..."
# Checkout master in source [in TO_REPO_NAME]
if [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then
    CHECKOUT_MASTER=1
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then
    CHECKOUT_MASTER=1
elif [[ "$FROM_REPO_NAME" = "lpub3d" ]]; then
    CHECKOUT_MASTER=1
fi
if [ -n "$(git status -s)" ]
then
    echo "  -WARNING - stashing unexpected uncommitted $TO_REPO_NAME changes..."
    git stash &>> $LOG
fi
[ -z "$CHECKOUT_MASTER" ] && git checkout master || true

# Checkout master in destination [in FROM_REPO_NAME] when not called from next_cutover (AUTO=)
if [ -z "$AUTO_MODE" ]
then
    if [ -n "$RELEASE_COMMIT" ]; then
        echo "$((COMMAND_COUNT += 1))-Create new version tag in $FROM_REPO_NAME repository"
        cd $HOME_DIR/$FROM_REPO_NAME
        rm -f *.log
        if [ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]; then git checkout master &>> $LOG; fi
        if GIT_DIR=./.git git rev-parse $LOCAL_TAG >/dev/null 2>&1; then git tag --delete $LOCAL_TAG; fi
        git tag -a $LOCAL_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
        git_tag="$(git tag -l -n $LOCAL_TAG)" && \
        [ -n "$git_tag" ] && echo "  -git tag $git_tag created."
		# Update config files with version from new tag
		./builds/utilities/hooks/pre-commit -ro && \
		./builds/utilities/hooks/pre-commit -rf && \
		rm -f *.log
		# Git append to amend the last commit to update config files with new version
		git add . &>> $LOG
		git commit --amend --no-edit &>> $LOG
		git log --stat &>> $LOG
    fi
    cd $HOME_DIR;
fi

echo && echo "   -Commit cutover completed: $COMMIT_MSG"
