#!/bin/bash
# Trevor SANDY
# Last Update October 03, 2020
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
#   Expected secrets folder contents (file names may vary; left-side key and token names are fixed):
#        - .sfdeploy_travis_rsa                                Soruceforge rsa key
#        - .github_api_keys                                    GitHub API token, Secure API keys and repository token keys :
#            - GITHUB_PROD_SECURE_API_KEY=<YOUR KEY HERE>      Production repository
#            - GITHUB_DEVL_SECURE_API_KEY=<YOUR KEY HERE>      Maintenance or development repository
#            - ...                                             Additional repository
#            - TRAVIS_PROD_GITHUB_TOKEN_KEY=<YOUR TOKEN HERE>  Production repository
#            - TRAVIS_DEVL_GITHUB_TOKEN_KEY=<YOUR TOKEN HERE>  Maintenance or development repository
#            - ...                                             Additional repository
#            - GITHUB_API_TOKEN=<YOUR TOKEN HERE>              GitHub global authorization
#        - .dropbox_oauth                                      Dropbox authorization key
#            - OAUTH_ACCESS_TOKEN=<YOUR TOKEN HERE>
#
# Execution Steps:
#
# Preq 1 of 3 Enable script execution [execute once] 
# $ chmod +x ci_cutover.sh && ./ci_cutover.sh
#
# Preq 2 of 3 Set 'Next' version number [execute once]
# $ sed 's/2.3.13/<next version>/g' -i ci_cutover.sh
#
# Preq 3 of 3 create and checkout a CUTOVER branch in the lpub3d-ci repository at the 
#   last production commit. [execute once]
#
# Step 1 of 3 from the CUTOVER branch add your master/dev branch commit by executing 
#   'reset to this commit' or 'cherrypick commit' [execute for each commit]
#
# Step 2 of 3 Production commits [execute for each commit except the final one for version change]
# $ env MSG="<commit> #No" OBS_CFG=yes TAG=v2.3.13 ./ci_cutover.sh
#
# Step 3 of 3 Final commit and production version change [execute once for version 
#   change [BE CAREFUL - THIS ADDS A TAG]
# $ env MSG="LPub3D v2.3.13" TAG=v2.3.13 REL=1 REV=no CNT=yes OBS_CFG=yes ./ci_cutover.sh
#
# Step 4 of 5 Copy README.txt and RELEASE_NOTES.html from 'lpub3d' back to 'lpub3d-ci'
# $ cp -f lpub3d/mainApp/docs/README.txt lpub3d-ci/mainApp/docs/
# $ cp -f lpub3d/mainApp/docs/RELEASE_NOTES.html lpub3d-ci/mainApp/docs/
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
#   - TAG: Git tag [Default=v2.3.13] - change as needed
#   - MSG: Git commit message [Default='Continuous integration cutover [build pkg]'] - change as needed
#   - FRESH: Clone a new instance of TO_REPO otherwise, only overwrite existing files [default=no]
#   - REV: Increment revision [Default=yes]
#   - CNT: Increment commit count [Default=yes]
#   - OBS_CFG: Set OBS config and README file updates [Default=no]
#   - REL: Release build, do not delete build tag [Default=no]
#   - AUTO: Do not prompt to continue after pausing at options status [Default=null]
#   - NOSTAT: Do not show the options_status [Default=null]
#
# Command Examples:
# $ chmod +x ci_cutover.sh && ./ci_cutover.sh
# $ env MSG="LPub3D pre-release [build pkg]" TAG=v2.3.13 ./ci_cutover.sh
# $ env MSG="LPub3D version 2.3.13" REL=1 REV=no OBS_CFG=yes ./ci_cutover.sh
# $ env FRESH=yes MSG="LPub3D version 2.3.13" TAG=v2.3.13 REV=no OBS_CFG=yes ./ci_cutover.sh
# $ env FRESH=yes MSG="LPub3D pre-release [build pkg]" TAG=v2.3.13 REV=no CNT=yes OBS_CFG=yes ./ci_cutover.sh
# $ env FRESH=yes MSG="Issue template and renderer logging updates" OBS_CFG=yes ./ci_cutover.sh
#
# Move from lpub3d-ci to lpub3d-obs repository
# $ env TO_REPO=lpub3d-obs MSG="Open Build Service Integration and Test" TAG=v2.3.13 ./ci_cutover.sh
#
# Move from lpub3d-ci to lpub3dnext repository
# Step 1 of 2 Change lpub3dnext branch to CUTOVER_CI
# Step 2 of 2 Final commit version change [execute once]
# $ env TO_REPO=lpub3dnext MSG="99b9b79e <commit message>" TAG=v2.3.13 REL=1 REV=no CNT=yes OBS_CFG=yes ./ci_cutover.sh
# $ env TO_REPO=lpub3dnext MSG="99b9b79e <commit message>" TAG=v2.3.13 OBS_CFG=no ./ci_cutover.sh
#
# Move from lpub3dnext to lpub3d-ci repository
# Step 1 of 2 Change lpub3d-ci branch to NEXT_IN
# Step 2 of 2 Maintenance commits [execute for each commit except the final one for version change]
# $ env TO_REPO=lpub3d-ci FROM_REPO=lpub3dnext MSG="64d28eb6-301019 Add PLI parts..." TAG=v2.3.13 OBS_CFG=yes ./ci_cutover.sh

SCRIPT_NAME=$0
SCRIPT_ARGS=$*
HOME_DIR=$PWD

NEXT_CUT=${AUTO:-}
NO_STATUS=${NOSTAT:-}
LOCAL_TAG=${TAG:-v2.3.13}
FRESH_BUILD=${FRESH:-}
INC_REVISION=${REV:-yes}
INC_COUNT=${CNT:-yes}
FORCE_CONFIG=${OBS_CFG:-no}
TO_REPO_NAME=${TO_REPO:-lpub3d}
FROM_REPO_NAME=${FROM_REPO:-lpub3d-ci}
RELEASE_COMMIT=${REL:-}
COMMIT_MSG="${MSG:-LPub3D ${TAG}}"

function options_status
{
    echo
    echo "--Command Options:"
    echo "--SCRIPT_NAME....$SCRIPT_NAME"
    echo "--FROM_REPO_NAME..$FROM_REPO_NAME"    
    echo "--TO_REPO_NAME....$TO_REPO_NAME"
    echo "--REPO_PATH.......$HOME_DIR"
    echo "--LOCAL_TAG.......$LOCAL_TAG"
    echo "--COMMIT_MSG......$COMMIT_MSG"
    [ -n "$FRESH_BUILD" ] && echo "--FRESH_BUILD.....YES" || true
    [ -n "$INC_REVISION" ] && echo "--INCREMENT_REV...YES" || true
    [ -n "$FORCE_CONFIG" ] && echo "--OBS_CONFIG......YES" || true
    [ -n "$RELEASE_COMMIT" ] && echo "--RELEASE_COMMIT..YES" || true    
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
if [ -z "$NO_STATUS" ]; then options_status; fi

# Confirmation
if [ -z "NEXT_CUT" ]
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
    echo && echo "--Attempting to remove old $TO_REPO_NAME instance..." 
    cd $HOME_DIR/$TO_REPO_NAME
    if [ -d ".git" ]
    then
        if [ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]; then
            git checkout master
        fi
        if [ -n "$(git status -s)" ]; then
            echo && echo "--ERROR - Uncommitted changes detected - exiting..."  
            cd $HOME_DIR
            exit 1
        fi
    fi
    cd $HOME_DIR
    rm -rf "$TO_REPO_NAME"
    if [ -d "$TO_REPO_NAME" ]; then
        echo && echo "--ERROR - Could not remove $TO_REPO_NAME - exiting..."
        exit 1
    else
        echo "--Removed $TO_REPO_NAME."
    fi
fi

# Clone new instance of lpub3d if old instance does not exist or was removed
if [ ! -d "$TO_REPO_NAME" ]; then
    echo && echo "1-Creating new $TO_REPO_NAME instance..."
    git clone https://github.com/trevorsandy/${TO_REPO_NAME}.git
else
    echo && echo "1-Updating existing $TO_REPO_NAME instance..."
fi

# Verify we are in the right source repository branch
cd $HOME_DIR/$FROM_REPO_NAME
if [ -n "$(git status -s)" ]; then
    echo && echo "--INFO - Stashing uncommitted $FROM_REPO_NAME changes..."  
    git stash
fi
if [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3d" ]]; then 
    START_BRANCH=CUTOVER_CI; 
elif [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then 
    START_BRANCH=CUTOVER_CI; 
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then 
    START_BRANCH=CUTOVER_NEXT;
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then 
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
    echo && echo "--INFO - Stashing uncommitted $TO_REPO_NAME changes..."  
    git stash
fi
if [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]];then 
    START_BRANCH=master
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then
    START_BRANCH=CUTOVER_CI
elif [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3dnext" ]];then 
    START_BRANCH=CUTOVER_NEXT
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then
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
    echo " -branch $START_BRANCH checked out."
else
    echo " -MY GOODNESS! Branch $START_BRANCH checkout failed - exiting."
    exit 1
fi
    
echo "2-Remove current $TO_REPO_NAME content except .git folder and .user file..."
find . -not -name '*.user' -not -path "./.git/*" -type f -exec rm -rf {} +

echo "3-Copy $FROM_REPO_NAME content to $TO_REPO_NAME except .git folder and .user file..." && cd ../$FROM_REPO_NAME
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
       -type f -exec cp -f --parents -t ../$TO_REPO_NAME {} +
cp -f ./.gitignore ../$TO_REPO_NAME

echo "4-Rename all files with '$FROM_REPO_NAME' in the name to '$TO_REPO_NAME'..." && cd ../$TO_REPO_NAME
for file in $(find . -type f -name "*${FROM_REPO_NAME}*" \
              -not -path "./.git*" \
              -not -path "./lclib*" \
              -not -path "./ldvlib*" \
              -not -path "./ldrawini*" \
              -not -path "./qslog*" \
              -not -path "./quazip*" \
              -not -path "./qsimpleupdater*" \
              -not -path "./builds/utilities/ci/next_cutover.sh" \
              )
do
    if   [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3d" ]]; then newFile=$(echo $file | sed s/-ci//g);
    elif [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then newFile=$(echo $file | sed s/-ci/next/g);
    elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then newFile=$(echo $file | sed s/next//g);
    elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then newFile=$(echo $file | sed s/next/-ci/g);
    elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dNext" ]]; then newFile=$(echo $file | sed s/lpub3d/lpub3dnext/g);
    elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then newFile=$(echo $file | sed s/lpub3d/lpub3d-ci/g);    
    fi
    mv -f "$file" "$newFile"
    [ -f "$newFile" ] && echo " -file changed: $newFile."
done
# rename project file with '$FROM_REPO_NAME' in the name to '$TO_REPO_NAME'..."
if [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then
    projFileName="LPub3D.pro"
    newProjFileName="LPub3DNext.pro"
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then
    projFileName="LPub3DNext.pro"
    newProjFileName="LPub3D.pro"
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then
    projFileName="LPub3D.pro"
    newProjFileName="LPub3DNext.pro"
fi
if [[ -f $projFileName && -z $newProjFileName && ($projFileName != $newProjFileName)]]; then
    mv -f "$projFileName" "$newProjFileName"
    [ -f "$newProjFileName" ] && echo " -file changed: $newProjFileName." || echo " -Error - $newProjFileName was not renamed." 
fi

echo "5-Change occurrences of '$FROM_REPO_NAME' to '$TO_REPO_NAME' in files..."
for file in $(find . -type f \
              -not -path "./.git/*" \
              -not -path "./mainApp/images*" \
              -not -path "./lclib*" \
              -not -path "./ldvlib*" \
              -not -path "./ldrawini*" \
              -not -path "./qslog*" \
              -not -path "./quazip*" \
              -not -path "./qsimpleupdater*" \
              -not -path "./builds/utilities/ci/travis/releases" \
              -not -path "./builds/utilities/ci/ci_cutover.sh" \
              -not -path "./builds/utilities/ci/next_cutover.sh" \
              )
do
    cat $file | grep -qE "$FROM_REPO_NAME" \
    && sed "s/${FROM_REPO_NAME}/${TO_REPO_NAME}/g" -i $file \
    && echo " -file updated: $file" || true
done

echo "6-Update 'dry-run' flag in sfdeploy.sh"
file=builds/utilities/ci/sfdeploy.sh
if [ "$TO_REPO_NAME" = "lpub3d" ]; then sed s/'--dry-run '//g -i $file; echo " -file $file updated.";
else sed s/'rsync --recursive'/'rsync --dry-run --recursive'/g -i $file; echo " -NOTICE - $TO_REPO_NAME not production, 'dry-run' preserved in $file.";
fi

echo "7-Update README.md Title"
file=README.md
if   [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3d" ]]; then sed s/' - Dev, CI, and Test'//g -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then s/' - Dev, CI, and Test'/' - Next Development'/g -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then sed s/' - Next Development'//g -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then s/' - Next Development'/' - Dev, CI, and Test'/g -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3dNext" ]]; then s/'# LPub3D'/'# LPub3D - Next Development'/g -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then s/'# LPub3D'/'# LPub3D - Dev, CI, and Test'/g -i $file; echo " -file $file updated.";  
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
source $secretsDir/.github_api_keys
if   [[ "$FROM_REPO_NAME" = "lpub3d-ci"  && "$TO_REPO_NAME" = "lpub3d" ]]; then sed s/${TRAVIS_DEVL_GITHUB_TOKEN_KEY}/${TRAVIS_PROD_GITHUB_TOKEN_KEY}/g -i $file; echo " -file $file updated with secure variable key.";
elif [[ "$FROM_REPO_NAME" = "lpub3d-ci"  && "$TO_REPO_NAME" = "lpub3dnext" ]]; then sed s/${TRAVIS_DEVL_GITHUB_TOKEN_KEY}/${TRAVIS_NEXT_GITHUB_TOKEN_KEY}/g -i $file; echo " -file $file updated with secure variable key.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then sed s/${TRAVIS_NEXT_GITHUB_TOKEN_KEY}/${TRAVIS_PROD_GITHUB_TOKEN_KEY}/g -i $file; echo " -file $file updated with secure variable key.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then sed s/${TRAVIS_NEXT_GITHUB_TOKEN_KEY}/${TRAVIS_DEVL_GITHUB_TOKEN_KEY}/g -i $file; echo " -file $file updated with secure variable key.";
elif [[ "$FROM_REPO_NAME" = "lpub3d"     && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then sed s/${TRAVIS_PROD_GITHUB_TOKEN_KEY}/${TRAVIS_DEVL_GITHUB_TOKEN_KEY}/g -i $file; echo " -file $file updated with secure variable key.";
elif [[ "$FROM_REPO_NAME" = "lpub3d"     && "$TO_REPO_NAME" = "lpub3dnext" ]]; then sed s/${TRAVIS_PROD_GITHUB_TOKEN_KEY}/${TRAVIS_NEXT_GITHUB_TOKEN_KEY}/g -i $file; echo " -file $file updated with secure variable key.";
else echo " -ERROR - file $file NOT updated.";
fi

echo "10-Replace leading spaces with tabs for LCLib files"
counter=0
for file in $(find ./lclib -type f -name '*.h' -o -name '*.cpp')
do
    echo "Converting $file..." &>> $LOG && \
    data=$(unexpand --first-only -t 4 $file) && \
    rm "$file" && \
    echo "$data" > "$file" && \
    counter=$((counter + 1)) || true
done
echo " -files updated: $counter"

echo "11-Change *.sh line endings from CRLF to LF"
for file in $(find . -type f -name *.sh)
do
    dos2unix -k $file &>> $LOG
done

echo "12-Change other line endings from CRLF to LF"
dos2unix -k builds/utilities/hooks/* &>> $LOG
dos2unix -k builds/utilities/create-dmg &>> $LOG
dos2unix -k builds/utilities/dmg-utils/* &>> $LOG
dos2unix -k builds/utilities/ci/travis/* &>> $LOG
dos2unix -k builds/utilities/mesa/* &>> $LOG
dos2unix -k builds/utilities/set-ldrawdir.command &>> $LOG
dos2unix -k builds/linux/docker-compose/* &>> $LOG
dos2unix -k builds/linux/docker-compose/dockerfiles/* &>> $LOG
dos2unix -k builds/linux/obs/* &>> $LOG
dos2unix -k builds/linux/obs/alldeps/* &>> $LOG
dos2unix -k builds/linux/obs/alldeps/debian/* &>> $LOG
dos2unix -k builds/linux/obs/debian/* &>> $LOG
dos2unix -k builds/linux/obs/debian/source/* &>> $LOG
dos2unix -k builds/macx/* &>> $LOG

echo "13-Change Windows script line endings from LF to CRLF"
unix2dos -k builds/windows/* &>> $LOG
unix2dos -k builds/utilities/CreateRenderers.bat &>> $LOG
unix2dos -k builds/utilities/update-config-files.bat &>> $LOG
unix2dos -k builds/utilities/nsis-scripts/* &>> $LOG
unix2dos -k builds/utilities/nsis-scripts/Include/* &>> $LOG

echo "14-Patch update-config-files.sh"
file=builds/utilities/update-config-files.sh
sed -e s/'.dsc   - add'/'.dsc      - add'/g \
    -e s/'.spec  - add'/'.spec     - add'/g -i $file \
&& echo " -file $file updated." \
|| echo " -ERROR - file $file NOT updated."

echo "15-update github api_key for Travis CI"
file=.travis.yml
if   [[ "$FROM_REPO_NAME" = "lpub3d-ci"  && "$TO_REPO_NAME" = "lpub3d" ]]; then sed "s,^        secure: ${GITHUB_DEVL_SECURE_API_KEY},        secure: ${GITHUB_PROD_SECURE_API_KEY}," -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d-ci"  && "$TO_REPO_NAME" = "lpub3dnext" ]]; then sed "s,^        secure: ${GITHUB_DEVL_SECURE_API_KEY},        secure: ${GITHUB_NEXT_SECURE_API_KEY}," -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d" ]]; then sed "s,^        secure: ${GITHUB_NEXT_SECURE_API_KEY},        secure: ${GITHUB_PROD_SECURE_API_KEY}," -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then sed "s,^        secure: ${GITHUB_NEXT_SECURE_API_KEY},        secure: ${GITHUB_DEVL_SECURE_API_KEY}," -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d"     && "$TO_REPO_NAME" = "lpub3dnext" ]]; then sed "s,^        secure: ${GITHUB_PROD_SECURE_API_KEY},        secure: ${GITHUB_NEXT_SECURE_API_KEY}," -i $file; echo " -file $file updated.";
elif [[ "$FROM_REPO_NAME" = "lpub3d"     && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then sed "s,^        secure: ${GITHUB_PROD_SECURE_API_KEY},        secure: ${GITHUB_DEVL_SECURE_API_KEY}," -i $file; echo " -file $file updated.";
else echo " -ERROR - file $file NOT updated.";
fi

echo "16-Add new files..."
git add . &>> $LOG
git reset HEAD 'mainApp/docs/README.txt'

echo "17-Create local tag in $TO_REPO_NAME repository"
if GIT_DIR=./.git git rev-parse $LOCAL_TAG >/dev/null 2>&1; then git tag --delete $LOCAL_TAG; fi
git tag -a $LOCAL_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
git_tag="$(git tag -l -n $LOCAL_TAG)" && \
[ -n "$git_tag" ] && echo " -git tag $git_tag created."

echo "18-Stage and commit changed files..."
cat << pbEOF >.git/COMMIT_EDITMSG
$COMMIT_MSG

pbEOF
env force=$FORCE_CONFIG inc_rev=$INC_REVISION inc_cnt=$INC_COUNT git commit -m "$COMMIT_MSG"

if [ -z "$RELEASE_COMMIT" ]; then
   echo "18-Delete local tag in $TO_REPO_NAME repository"
   git tag --delete $LOCAL_TAG
   rm -f update-config-files.sh.log
fi

echo "19-Restore repository checked out state..."
# Checkout master in source [in TO_REPO_NAME]
if [[ "$FROM_REPO_NAME" = "lpub3d-ci" && "$TO_REPO_NAME" = "lpub3dnext" ]]; then
    CHECKOUT_MASTER=1
elif [[ "$FROM_REPO_NAME" = "lpub3dnext" && "$TO_REPO_NAME" = "lpub3d-ci" ]]; then
    CHECKOUT_MASTER=1
elif [[ "$FROM_REPO_NAME" = "lpub3d" ]]; then
    CHECKOUT_MASTER=1
fi
[ -z "$CHECKOUT_MASTER" ] && git checkout master || true

# Checkout master in destination [in FROM_REPO_NAME]
cd $HOME_DIR/$FROM_REPO_NAME
if [ -z "$NEXT_CUT" ]
then
    git checkout master
    if [ -n "$RELEASE_COMMIT" ]; then
        echo "19-Create local tag in $FROM_REPO_NAME repository" 
        if GIT_DIR=./.git git rev-parse $LOCAL_TAG >/dev/null 2>&1; then git tag --delete $LOCAL_TAG; fi
        git tag -a $LOCAL_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
        git_tag="$(git tag -l -n $LOCAL_TAG)" && \
        [ -n "$git_tag" ] && echo " -git tag $git_tag created."
    fi   
    cd $HOME_DIR;
fi

echo && echo "   -Commit cutover completed: $COMMIT_MSG"
