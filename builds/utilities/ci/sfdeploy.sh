#!/bin/bash
#
# Deploy LPub3D assets to Sourceforge.net using OpenSSH and rsync
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: Jan 14, 2023
#  Copyright (C) 2017 - 2024 by Trevor SANDY
#
#  Note: this script requires SSH host public/private keys

set +x

# Capture elapsed time - reset BASH time counter
SECONDS=0
SfElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "SfD${ME:3} Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap SfElapsedTime EXIT

ME="sfdeploy"
[ "$(basename $0)" != "${ME}.sh" ] && WRITE_LOG=false || \
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")" # not sourced
CWD=`pwd`

if [ "$APPVEYOR" != "True" ]; then
  # logging stuff - increment log file name
  f="${CWD}/CreateSourceforgeD${ME:3}"
  [ "$TRAVIS" = "true" ] && \
  ext=".${LP3D_ASSET_EXT}.log" || \
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
fi

echo && echo "Start S${ME:1} execution" && echo

# set working directory
sfParent_dir=${PWD##*/}
if  [ "$sfParent_dir" = "ci" ]; then
  sfChkdir="$(realpath ../../../../)"
  [ -d "$chkdir" ] && sfWD=$sfChkdir || sfWD=$PWD
else
  sfWD=$PWD
fi

# set remote host update and download paths
LP3D_SF_REMOTE_HOST="frs.sourceforge.net"  # 216.34.181.57
LP3D_SF_UDPATE_CONNECT="trevorsandy@${LP3D_SF_REMOTE_HOST}:/home/project-web/lpub3d/htdocs"
LP3D_SF_DOWNLOAD_CONNECT="trevorsandy@${LP3D_SF_REMOTE_HOST}:/home/pfs/project/lpub3d"

if [ "$APPVEYOR" = "True" ]; then
  echo && echo "- Deploying Appveyor release assets to Sourceforge.net..."

  # load lp3d* environment variables into bash
  if [ -f "builds/utilities/ci/set_bash_vars.sh" ]; then
    echo && echo "- source set_bash_vars.sh..."
    source builds/utilities/ci/set_bash_vars.sh && echo
  fi

  # set host private key
  LP3D_HOST_SSH_KEY=".sfdeploy_appveyor_rsa"

  # move host private key to tmp folder
  if [ -f "builds/utilities/ci/secure/$LP3D_HOST_SSH_KEY" ]; then
    mv -f "builds/utilities/ci/secure/$LP3D_HOST_SSH_KEY" "/tmp/$LP3D_HOST_SSH_KEY"
  else
    echo "  ERROR - builds/utilities/ci/secure/$LP3D_HOST_SSH_KEY was not found."
    export LP3D_SF_DEPLOY_ABORT=true;
  fi
elif [ "$TRAVIS" = "true" ]; then
  echo && echo "Deploying Travis release assets to Sourceforge.net..."

  # set host private key
  LP3D_HOST_SSH_KEY=".sfdeploy_travis_rsa"
elif [ "$GITHUB" = "true" ]; then
  echo && echo "Deploying Github actions release assets to Sourceforge.net..."
fi

# add remote host public key to ~/.ssh/known_hosts - prevent interactive prompt
echo
[ ! -d ~/.ssh ] && mkdir -p ~/.ssh && touch ~/.ssh/known_hosts || \
[ ! -f ~/.ssh/known_hosts ] && touch ~/.ssh/known_hosts || true
[ -z "$(ssh-keygen -F $LP3D_SF_REMOTE_HOST)" ] && \
ssh-keyscan -H $LP3D_SF_REMOTE_HOST >> ~/.ssh/known_hosts && \
echo "Remote host $LP3D_SF_REMOTE_HOST public key added to ~/.ssh/known_hosts." || \
echo "Remote host $LP3D_SF_REMOTE_HOST public key exist in ~/.ssh/known_hosts."

# where are we working from
echo && echo "  WORKING DIRECTORY............[$sfWD]" && echo

# add host private key to ssh-agent
eval "$(ssh-agent -s)"
if [ -s "${LP3D_HOST_SSH_KEY}" ]; then
  ssh-add $LP3D_HOST_SSH_KEY
elif [ -f "/tmp/${LP3D_HOST_SSH_KEY}" ]; then
  chmod 600 /tmp/$LP3D_HOST_SSH_KEY
  ssh-add /tmp/$LP3D_HOST_SSH_KEY
else
  echo "ERROR - RSA key not found at $LP3D_HOST_SSH_KEY - cannot perform transfers"
  export LP3D_SF_DEPLOY_ABORT=true
fi

# DEBUG SSH command
# ssh -vvv -i $LP3D_HOST_SSH_KEY $LP3D_SF_REMOTE_HOST

# upload assets
if [ -z "$LP3D_SF_DEPLOY_ABORT" ]; then
  echo
  LP3D_SF_DEPLOY_OPTIONS="NONE"
  if [ "$APPVEYOR" = "True" ]; then
    LP3D_SF_DEPLOY_OPTIONS="UDPATE DOWNLOAD"
    if [ "$LP3D_CONTINUOUS_BUILD_PKG" = "true" ]; then
      LP3D_SF_FOLDER="Continuous"
    else
      LP3D_SF_FOLDER="$LP3D_VERSION"
    fi
  else
    if [ "$TRAVIS" = "true" ]; then
      echo "  LP3D_ASSET_EXTENSION.........[${LP3D_ASSET_EXT}]"
      if [ -z "${LP3D_PROJECT_NAME}" ]; then
        IFS='/' read -ra LP3D_SLUGS <<< "$TRAVIS_REPO_SLUG"; unset IFS;
        LP3D_PROJECT_NAME="${LP3D_SLUGS[1]}"
      fi
      [ -d "$LP3D_UPDATE_ASSETS" ] && \
      LP3D_SF_DEPLOY_OPTIONS="UDPATE" ||
      LP3D_SF_DEPLOY_OPTIONS="DOWNLOAD"
    fi
    [ "$GITHUB" = "true" ] && \
    LP3D_SF_DEPLOY_OPTIONS="UDPATE DOWNLOAD" || :
    if [ "$LP3D_DEPLOY_PKG" = "yes" ]; then
      LP3D_SF_FOLDER="$LP3D_VERSION"
    else
      LP3D_SF_FOLDER="Continuous"
    fi
  fi
  echo "  LP3D_PROJECT_NAME............[${LP3D_PROJECT_NAME}]"
  echo "  LP3D_SF_DEPLOY_OPTIONS.......[$LP3D_SF_DEPLOY_OPTIONS]"
  echo "  LP3D_SF_DOWNLOAD_FOLDER......[$LP3D_SF_FOLDER]"
  for OPTION in $LP3D_SF_DEPLOY_OPTIONS; do
    case $OPTION in
    UDPATE)
      # Verify release files in the Update directory
      if [ -d "$LP3D_UPDATE_ASSETS" ]; then
        if [ -n "$(find "$LP3D_UPDATE_ASSETS" -maxdepth 0 -type d -empty 2>/dev/null)" ]; then
          echo && echo "WARNING - Folder $LP3D_UPDATE_ASSETS is empty. $OPTION assets deploy aborted." && echo
        else
          echo && echo "- $OPTION Assets:" && \
          find "$LP3D_UPDATE_ASSETS" -type f
          echo && echo "Executing rsync upload for $(basename $LP3D_UPDATE_ASSETS)..." && echo
          rsync --recursive --verbose --compress $LP3D_UPDATE_ASSETS/ $LP3D_SF_UDPATE_CONNECT/
        fi
      else
        echo && echo "WARNING - Folder ${LP3D_UPDATE_ASSETS} was not found."
      fi
      ;;
    DOWNLOAD)
      # Verify release files in the Download directory
      if [ -n "$(find "$LP3D_DOWNLOAD_ASSETS" -maxdepth 0 -type d -empty 2>/dev/null)" ]; then
        echo && echo "WARNING - Folder $LP3D_DOWNLOAD_ASSETS is empty. $OPTION assets deploy aborted."
      else
        echo && echo "- $OPTION Assets:" && \
        find "$LP3D_DOWNLOAD_ASSETS" -type f -not -path "$LP3D_UPDATE_ASSETS/*"
        if [ "$GITHUB" = "true" ]; then
          echo && echo "Executing rsync upload for $(basename $LP3D_DOWNLOAD_ASSETS)..." && echo
          rsync --recursive --verbose --compress --delete-before \
          --include={'*.exe','*.zip','*.deb','*.rpm','*.zst','*.dmg','*.AppImage','*.sha512','*.sig','*.html','*.txt'} --exclude '*' \
          $LP3D_DOWNLOAD_ASSETS/ $LP3D_SF_DOWNLOAD_CONNECT/$LP3D_SF_FOLDER/
        elif [ "$APPVEYOR" = "True" ]; then
          echo && echo "Executing rsync upload for $(basename $LP3D_DOWNLOAD_ASSETS)..." && echo
          rsync --recursive --verbose --compress --delete-before \
          --include={'*.exe','*.zip','*.html','*.txt','*.sha512'} --exclude '*' \
          $LP3D_DOWNLOAD_ASSETS/ $LP3D_SF_DOWNLOAD_CONNECT/$LP3D_SF_FOLDER/
        else
          echo && echo "Executing rsync upload for file type ${LP3D_ASSET_EXT}..." && echo
          rsync --recursive --verbose --compress --delete-before \
          --include "*${LP3D_ASSET_EXT}*" --exclude '*' \
          $LP3D_DOWNLOAD_ASSETS/ $LP3D_SF_DOWNLOAD_CONNECT/$LP3D_SF_FOLDER/
        fi
      fi
      [ $? -ne 0 ] && exit 1 || :
      ;;
    esac
  done
fi

exit 0
