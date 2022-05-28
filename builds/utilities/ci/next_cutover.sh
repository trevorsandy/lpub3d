#!/bin/bash
# Trevor SANDY
# Last Update May 27, 2022
#
# Purpose:
# This script will automate the 'cutover' of a range of commits from development [lpub3dnext] or maintenance [lpub3d-ci] repository to production [lpub3d].
# Commit range is processed from oldest to newest using either reset or cherry-pick as specified by user performing cutover.
# The starting commit 'COMMIT' is exclusive, meaning it is not included in the commits passed to the receiving repository.
#
# Setup:
# For successful execution it and ci_cutover.sh must be placed at the root of the repositories, for example:
#   ./lpub3d                - production repository
#   ./lpub3d-ci             - maintenance repository
#   ./lpub3dnext            - development repository
#   ./ci_cutover.sh         - single commit cutover script
#   ./next_cutover.sh       - range of commits cutover script
#
# Environment variables:
#   - DRY_RUN: Do not perform cutover [Default=null]
#   - TAG: Release version [Default=2.4.0']
#   - TO_REPO: Production or maintenance repository [default=lpub3d]
#   - FROM_REPO: Development repository [default=lpub3dnext]
#   - COMMIT: Starting commit - exclusive (the next commit will be processed)[Default=null]
#   - BRANCH: Working cutover development branch [Default=CUTOVER_CI]
#   - RELEASE: Create a release commit, preserve build tag, on the last commit [Default=null]
#   - CLONE: Clone a new repository of TO_REPO at the first commit [Default=null]
#   - CHERRYPICK: Cherry-pick commits as we process the range, otherwise use reset [Default=null]
#
# Execution Steps:
#
# Add production remote to development repository
# Determine version
# Identify starting commit
# Execute:
#   $ chmod +x next_cutover.sh
#   $ env FROM_REPO=lpub3dnext TO_REPO=lpub3d TAG=v2.4.4 CLONE=1 RELEASE=1 DRY_RUN=1 COMMIT=f61eb11778e1fb206f ./next_cutover.sh
#
# NOTE: Set RELEASE=1 and TAG=<new tag> if this cutover will end with a new version tag
#       You can set TAG=<current tag> (or no tag at all) if this cutover will not end in a new tagged version
#
# Command Examples:
#   $ env FROM_REPO=lpub3dnext TO_REPO=lpub3d TAG=v2.4.4 CLONE=1 RELEASE=1 COMMIT=9711c7147c973c ./next_cutover.sh
#   $ env FROM_REPO=lpub3dnext TO_REPO=lpub3d TAG=v2.4.4 CLONE=1 RELEASE=1 DRY_RUN=1 COMMIT=9711c7147c973c ./next_cutover.sh
#   $ env FROM_REPO=lpub3dnext TO_REPO=lpub3d DRY_RUN=1 COMMIT=9711c7147c973c ./next_cutover.sh
#   $ env FROM_REPO=lpub3dnext TO_REPO=lpub3d TAG=v2.4.4 BRANCH=CUTOVER_CI CLONE=1 RELEASE=1 COMMIT=9711c7147c973c ./next_cutover.sh
#
# Useful DEBUG Commands:
# $ COMMIT_LIST_FILE="../cutover_commits.lst"
# $ COMMIT_LIST=$(git rev-list --reverse $START_COMMIT..HEAD 2>&1 | tee $COMMIT_LIST_FILE)
# $ COMMIT_LIST=$(git rev-list --reverse $START_COMMIT^..HEAD 2>&1 | tee $COMMIT_LIST_FILE) # includes START_COMMIT
# $ echo "$COMMIT_LIST" 2>&1 | tee "$COMMIT_LIST_FILE"
#
# $ echo && echo "START COMMIT: $START_COMMIT"
# $ COMMIT_LIST_FILE="../cutover_commits.lst"
# $ echo "---------------------------"
# $ [ -z "$COMMIT_LIST" ] && echo "   No commits found" || \
# $ echo "$COMMIT_LIST" 2>&1 | tee "$COMMIT_LIST_FILE"
#
# $ COMMIT_DESCRIPTION_LIST_FILE="../cutover_commits_descriptions.lst"
# $ COMMIT_DESCRIPTIONS=$(git log master --reverse --pretty=%B $START_COMMIT..HEAD | sed '/^$/d')
# $ echo "$COMMIT_DESCRIPTIONS" 2>&1 | tee "$COMMIT_DESCRIPTION_LIST_FILE"
#
# $ echo && echo "START COMMIT: $START_COMMIT"
# $ COMMIT_LIST_FILE="../cutover_commits.lst"
# $ echo "---------------------------"
# $ [ -z "$COMMIT_LIST" ] && echo "   No commits found" || \
# $ echo "$COMMIT_LIST" 2>&1 | tee "$COMMIT_LIST_FILE"
#
#

SCRIPT_NAME=$0
SCRIPT_ARGS=$*
HOME_DIR=$PWD

DO_DRY_RUN=${DRY_RUN:-}
VER_TAG=${TAG:-}
TO_REPO_NAME=${TO_REPO:-lpub3d-ci}
FROM_REPO_NAME=${FROM_REPO:-lpub3dnext}
START_COMMIT=${COMMIT:-}
START_BRANCH=${BRANCH:-CUTOVER_CI}
RELEASE_BUILD=${RELEASE:-}
FRESH_BUILD=${CLONE:-}
USE_CHERRY_PICK=${CHERRYPICK:-}
COMMIT_COUNT=0
COUNTER=0
SECONDS=0

# Set HOLD_VER_TAG to VER_TAG
HOLD_VER_TAG=${VER_TAG}
# Set VER_TAG to current tag, VER_TAG will revert to HOLD_VER_TAG on last commit
VER_TAG="$(cd $HOME_DIR/$FROM_REPO_NAME ; git describe --abbrev=0)"

FinishElapsedTime() {
  [ -z "$1" ] && ACTION=Finished || ACTION=$1
  ACTION="$ACTION - $COUNTER of $COMMIT_COUNT commits processed"

  # Elapsed execution time
  ELAPSED="Elapsed cutover time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo
  echo "------------------------------------------------------------------------------"
  echo "-4 Cutover $ACTION - see $LOG for details."
  echo "-- $ELAPSED"
  echo "------------------------------------------------------------------------------"
  echo
}

function options_status
{
    echo
    echo "--Command Options:"
    echo "--SCRIPT_NAME....$SCRIPT_NAME"
    echo "--REPO_PATH.......$HOME_DIR"
    echo "--NEW_VER_TAG.....$HOLD_VER_TAG"
    echo "--CURRENT_TAG.....$VER_TAG"
    echo "--FROM_REPO_NAME..$FROM_REPO_NAME"
    echo "--TO_REPO_NAME....$TO_REPO_NAME"
    echo "--START_COMMIT....$START_COMMIT"
    echo "--START_BRANCH....$START_BRANCH"
    [ -n "$FRESH_BUILD" ] && echo "--FRESH_BUILD.....YES" || true
    [ -n "$USE_CHERRY_PICK" ] && echo "--USE_CHERRY_PICK.YES" || true
    [ -n "$DO_DRY_RUN" ] && echo "--DO_DRY_RUN......YES" || true
    [ -n "$SCRIPT_ARGS" ] && echo "--SCRIPT_ARGS.....$SCRIPT_ARGS" || true
    echo
}

function get_elided_string()
{
    v=$1
    if [[ ${#v} > 40 ]]
    then
        echo "${v:0:33}...${v:(-4)}"
    else
        echo "$v"
    fi
}

function get_commit_count()
{
    echo $#;
}

ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
CWD=`pwd`
f="${CWD}/$ME"
ext=".log"
if [[ -e "$f$ext" ]]
then
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

# Check for required argument
function show_options_status
{
    SCRIPT_ARGS=$1
    options_status
    exit 1
}

if [ -z "$START_COMMIT" ]
then
    show_options_status "-- No starting commit specified - show options and exit"
fi

options_status

cd $HOME_DIR/$FROM_REPO_NAME

echo && echo "-1 Checkout $START_BRANCH branch at starting cutover commit $START_COMMIT..."
if [ -n "$(git status -s)" ]
then
    echo && echo "--INFO - Stashing uncommitted $FROM_REPO_NAME changes..."
    git stash &>> $LOG
fi
if [ -n "$(git show-ref refs/heads/$START_BRANCH)" ]
then # start branch exist
    if [ "$(git rev-parse --abbrev-ref HEAD)" != "$START_BRANCH" ]
    then # start branch not checked out so do so
        git checkout $START_BRANCH &>> $LOG
    fi
    git reset --hard $START_COMMIT &>> $LOG
else # start branch does not exist so create it and checkout
    git checkout -b $START_BRANCH $START_COMMIT &>> $LOG
fi
if [ "$(git rev-parse --abbrev-ref HEAD)" != "$START_BRANCH" ]
then # Test that all is ok
    echo "   -WHOA! branch $START_BRANCH checkout failed - exiting."
    FinishElapsedTime "Terminated"
    exit 1
fi

echo && echo "-2 Prepare cutover commits from oldest to newest to be passed to production..."
COUNTER=0
COMMIT_LIST=$(git log master --reverse --pretty=%H $START_COMMIT..HEAD)
COMMIT_COUNT=$(get_commit_count $COMMIT_LIST)
echo && echo "-3 $COMMIT_COUNT cutover commits to be processed..."

# Confirmation
echo
sleep 1s && read -p "  Do you want to continue (y/n)? " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
  if [ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]; then
    git checkout master &>> $LOG
  fi
  [[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1 # handle exits from shell or function but don't exit interactive shell
fi

LAST_COMMIT=none

for commit in $COMMIT_LIST;
do

    cd $HOME_DIR/$FROM_REPO_NAME
    if [ "$(git rev-parse --abbrev-ref HEAD)" != "$START_BRANCH" ]
    then
        git checkout $START_BRANCH &>> $LOG
    fi

    SHORT_COMMIT=$(cut -c-10 <<< "$commit")
    COMMIT_DESC=$(git show -s --format=%s $commit)
    COMMIT_DESC_ELIDED=$(get_elided_string "$COMMIT_DESC")
    COUNT=$((COUNTER + 1))

    if [ -n "$USE_CHERRY_PICK" ]
    then
        echo && echo "   -Cherry-picking commit $COUNT of $COMMIT_COUNT: $SHORT_COMMIT - $COMMIT_DESC_ELIDED..."
        git cherry-pick $commit &>> $LOG
    else
        echo && echo "   -Setting commit $COUNT of $COMMIT_COUNT: $SHORT_COMMIT - $COMMIT_DESC_ELIDED..."
        git reset --hard $commit &>> $LOG
    fi
    RETURN_CODE=$?
    if [[ $RETURN_CODE != 0 ]]      # return code
    then
        echo
        if [ -n "$USE_CHERRY_PICK" ]
        then
            echo "-- WHOA! Cherry-pick '$SHORT_COMMIT' failed."
        else
            echo "-- WHOA! Reset '$SHORT_COMMIT' failed."
        fi
        echo "-- Return code was $RETURN_CODE - exiting!"
        FinishElapsedTime "Terminated"
        exit 1
    fi

    cd $HOME_DIR

    if [[ $COUNT = 1 ]]  # First commit - show status
    then
        echo "   -Cutover command: FROM_REPO=\"${FROM_REPO_NAME}\" TO_REPO=\"${TO_REPO_NAME}\" MSG=\"${COMMIT_DESC_ELIDED}\" TAG=$VER_TAG AUTO=1 OBS_CFG=yes FRESH=$FRESH_BUILD"
        if [ -z "$DO_DRY_RUN" ]
        then
            env FROM_REPO="${FROM_REPO_NAME}" TO_REPO="${TO_REPO_NAME}" MSG="${COMMIT_DESC}" TAG=$VER_TAG AUTO=1 OBS_CFG=yes FRESH=$FRESH_BUILD ./ci_cutover.sh /dev/null 2>&1
        fi
    elif [[ $((COUNTER + 1)) = $COMMIT_COUNT ]] # Last commit - use VER_TAG and show status
    then
        [[ "$VER_TAG" != "$HOLD_VER_TAG" && -n "$RELEASE_BUILD" ]] && VER_TAG=$HOLD_VER_TAG || :
        echo "   -Cutover command: FROM_REPO=\"${FROM_REPO_NAME}\" TO_REPO=\"${TO_REPO_NAME}\" MSG=\"${COMMIT_DESC_ELIDED}\" TAG=$VER_TAG AUTO=1 OBS_CFG=yes REL=$RELEASE_BUILD"
        if [ -z "$DO_DRY_RUN" ]
        then
            env FROM_REPO="${FROM_REPO_NAME}" TO_REPO="${TO_REPO_NAME}" MSG="${COMMIT_DESC}" TAG=$VER_TAG AUTO=1 OBS_CFG=yes REL=$RELEASE_BUILD ./ci_cutover.sh /dev/null 2>&1
            echo && echo "   -Release commit, create local tag in $FROM_REPO_NAME repository"
            cd $HOME_DIR/$FROM_REPO_NAME
            if [ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]; then git checkout master &>> $LOG; fi
            if GIT_DIR=./.git git rev-parse $VER_TAG >/dev/null 2>&1; then git tag --delete $VER_TAG &>> $LOG; fi
            git tag -a $VER_TAG -m "LPub3D $(date +%d.%m.%Y)" && \
            GIT_TAG="$(git tag -l -n $VER_TAG)" && \
            [ -n "$GIT_TAG" ] && echo "   -Release tag $GIT_TAG created."
        fi
    else
        echo "   -Cutover command: FROM_REPO=\"${FROM_REPO_NAME}\" TO_REPO=\"${TO_REPO_NAME}\" MSG=\"${COMMIT_DESC_ELIDED}\" TAG=$VER_TAG AUTO=1 OBS_CFG=yes NOSTAT=1"
        if [ -z "$DO_DRY_RUN" ]
        then
            env FROM_REPO="${FROM_REPO_NAME}" TO_REPO="${TO_REPO_NAME}" MSG="${COMMIT_DESC}" TAG=$VER_TAG AUTO=1 OBS_CFG=yes NOSTAT=1 ./ci_cutover.sh /dev/null 2>&1
        fi
    fi
    RETURN_CODE=$?
    if [[ $RETURN_CODE != 0 ]]      # return code
    then
        echo
        echo "-- HELLO! Commit '$SHORT_COMMIT' cutover failed."
        echo "-- Return code was $RETURN_CODE - exiting!"
        echo "-- Last successful commit was $LAST_COMMIT"
        FinishElapsedTime "Terminated"
        exit 1
    fi

    # Increment counter
    COUNTER=$((COUNTER + 1))

    # Last successful commit
    LAST_COMMIT=$commit
done

cd $HOME_DIR;

# Elapsed execution time
FinishElapsedTime
