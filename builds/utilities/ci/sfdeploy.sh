#!/bin/bash
#
# Deploy LPub3D assets to Sourceforge.net using OpenSSH and rsync
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: Aug 07, 2019
#  Copyright (c) 2017 - 2019 by Trevor SANDY
#
#  Note: this script requires SSH host public/private keys

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
  echo && echo "- source set_bash_vars.sh..."
  source builds/utilities/ci/set_bash_vars.sh && echo

  # set host private key
  LP3D_HOST_RSA_KEY=".sfdeploy_appveyor_rsa"

  # move host private key to tmp folder
  if [ -f "builds/utilities/ci/secure/$LP3D_HOST_RSA_KEY" ]; then
    mv -f "builds/utilities/ci/secure/$LP3D_HOST_RSA_KEY" "/tmp/$LP3D_HOST_RSA_KEY"
  else
    echo "  ERROR - builds/utilities/ci/secure/$LP3D_HOST_RSA_KEY was not found."
    export LP3D_SF_DEPLOY_ABORT=true;
  fi

  # add host public key to .ssh/known_hosts - prevent interactive prompt
  [ ! -d ~/.ssh ] && mkdir -p ~/.ssh && touch ~/.ssh/known_hosts || \
  [ ! -f ~/.ssh/known_hosts ] && touch ~/.ssh/known_hosts || true
  [ -z `ssh-keygen -F $LP3D_SF_REMOTE_HOST` ] && ssh-keyscan -H $LP3D_SF_REMOTE_HOST >> ~/.ssh/known_hosts || \
  echo  "Public key for remote host $LP3D_SF_REMOTE_HOST exist in .ssh/known_hosts."
elif [ "$TRAVIS" = "true" ]; then
  echo && echo "Deploying Travis release assets to Sourceforge.net..."

  # set host private key
  LP3D_HOST_RSA_KEY=".sfdeploy_travis_rsa"
fi

# where are we working from
echo && echo "  WORKING DIRECTORY............[$sfWD]" && echo

# add host private key to ssh-agent
if [ -f "/tmp/$LP3D_HOST_RSA_KEY" ]; then
  eval "$(ssh-agent -s)"
  chmod 600 /tmp/$LP3D_HOST_RSA_KEY
  ssh-add /tmp/$LP3D_HOST_RSA_KEY
else
  echo "ERROR - /tmp/$LP3D_HOST_RSA_KEY not found - cannot perform Sourceforge.net transfers"
  export LP3D_SF_DEPLOY_ABORT=true
fi

# upload assets
if [ -z "$LP3D_SF_DEPLOY_ABORT" ]; then
  echo
  LP3D_SF_DEPLOY_OPTIONS="NONE"
  if [[ "$TRAVIS_OS_NAME" == "linux" || "$TRAVIS_OS_NAME" == "osx" ]]; then
     IFS='/' read -ra LP3D_SLUG_PARTS <<< $TRAVIS_REPO_SLUG; unset IFS;
     echo "  TRAVIS_PROJECT_NAME..........[${LP3D_SLUG_PARTS[1]}]"
     if [ "${LP3D_SLUG_PARTS[1]}" = "lpub3d" ]; then
       LP3D_SF_DEPLOY_OPTIONS="DOWNLOAD"
       if [ "$LP3D_DEPLOY_PKG" != "yes" ];then
         LP3D_SF_FOLDER="Continuous"
       else
         LP3D_SF_FOLDER="$LP3D_VERSION"
       fi
     fi
  elif [ "$APPVEYOR" = "True" ]; then
    if [ "$APPVEYOR_PROJECT_NAME" = "lpub3d" ]; then
      echo "  APPVEYOR_PROJECT_NAME........[$APPVEYOR_PROJECT_NAME]"
      LP3D_SF_DEPLOY_OPTIONS="UDPATE DOWNLOAD"
      if [ "$LP3D_CONTINUOUS_BUILD_PKG" = "true" ]; then
        LP3D_SF_FOLDER="Continuous"
      else
        LP3D_SF_FOLDER="$LP3D_VERSION"
      fi
    fi
  fi
  echo "  LP3D_SF_DEPLOY_OPTIONS.......[$LP3D_SF_DEPLOY_OPTIONS]"
  echo "  LP3D_SF_DOWNLOAD_FOLDER......[$LP3D_SF_FOLDER]"
  for OPTION in $LP3D_SF_DEPLOY_OPTIONS; do
    case $OPTION in
    UDPATE)
      # Verify release files in the Update directory
      if [ -n "$(find "$LP3D_UPDATE_ASSETS" -maxdepth 0 -type d -empty 2>/dev/null)" ]; then
        echo && echo "$LP3D_UPDATE_ASSETS is empty. Sourceforge.net update assets deploy aborted."
      else
        echo && echo "- Update Release Assets:" && find $LP3D_UPDATE_ASSETS -type f && echo
        rsync --recursive --verbose $LP3D_UPDATE_ASSETS/* $LP3D_SF_UDPATE_CONNECT/
      fi
      ;;
    DOWNLOAD)
      # Verify release files in the Download directory
      if [ -n "$(find "$LP3D_DOWNLOAD_ASSETS" -maxdepth 0 -type d -empty 2>/dev/null)" ]; then
        echo && echo "$LP3D_DOWNLOAD_ASSETS is empty. Sourceforge.net download assets deploy aborted."
      else
        echo && echo "- Download Release Assets:" && find $LP3D_DOWNLOAD_ASSETS -type f && echo
        rsync --recursive --verbose $LP3D_DOWNLOAD_ASSETS/* $LP3D_SF_DOWNLOAD_CONNECT/$LP3D_SF_FOLDER/
      fi
      ;;
    esac
  done
fi

