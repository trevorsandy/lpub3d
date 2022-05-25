#!/bin/bash
# Trevor SANDY
# Last Update July 07, 2021
#
# This script is called from .github/workflows/build.yml
#
# Run command:
# bash -ex builds/utilities/ci/github/build-deploy.sh

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  set +x
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "${0##*/} Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

trap FinishElapsedTime EXIT

# Publish assets to Sourceforge
PublishToSourceforge() {
  if [ -z "$LP3D_SF_DEPLOY_ABORT" ]; then
    ( chmod a+x builds/utilities/ci/sfdeploy.sh && ./builds/utilities/ci/sfdeploy.sh ) >$p.out 2>&1 && mv $p.out $p.ok 
    [ ! -f $p.ok ] && \
    echo "WARNING - Sourceforge upload failed." && tail -80 $p.out || cat $p.ok
    if [ -n "${LP3D_UPDATE_ASSETS}" ]; then
      rm -rf ${LP3D_UPDATE_ASSETS}
      unset LP3D_UPDATE_ASSETS
    fi
  fi
}

# GPG sign .sha512 files and publish assets to Github
PublishToGitHub() {
  if [ -f "${LP3D_RELEASE}" ]; then
    case ${LP3D_ASSET_EXT} in
      ".exe"|".zip"|".deb"|".rpm"|".zst"|".dmg"|".AppImage")
      if [ -f "${LP3D_RELEASE}.sha512" ]; then
        # Generate hash file signature
        if [[ ! -f $s.out && -f "${LP3D_ASSET}.sha512" ]]; then
          echo && echo "- Signing ${LP3D_ASSET}.sha512 file..."
          ( gpg --home "${LP3D_GPGHOME}" --clearsign -u ${GPG_SIGN_KEY_ALIAS} \
            -o "${LP3D_RELEASE}.sha512.sig" "${LP3D_RELEASE}.sha512" \
          ) >$s.out 2>&1 && rm $s.out
          if [ ! -f $s.out ]; then
            # Publish hash signature file
            ( bash upload.sh "${LP3D_RELEASE}.sha512.sig" ) >$p.out 2>&1 && rm $p.out
            [ -f $p.out ] && \
            echo "WARNING - ${LP3D_RELEASE}.sha512.sig GitHub upload failed." && tail -80 $p.out || :
          else
            echo "WARNING - Create signature file failed." && tail -80 $s.out
          fi
        fi
        # Publish hash file
        ( bash upload.sh "${LP3D_RELEASE}.sha512" ) >$p.out 2>&1 && rm $p.out
        [ -f $p.out ] && \
        echo "WARNING - ${LP3D_RELEASE}.sha512 GitHub upload failed." && tail -80 $p.out || :
      else
        echo "WARNING - ${LP3D_RELEASE}.sha512 file not found."
      fi
      ;;
    esac
    # Publish build asset
    ( bash upload.sh "${LP3D_RELEASE}" ) >$p.out 2>&1 && rm $p.out
    [ -f $p.out ] && \
    echo "WARNING - ${LP3D_RELEASE} GitHub upload failed." && tail -80 $p.out || :
  else
    echo "WARNING - ${LP3D_RELEASE} file not found."
  fi
}

# Set variables
declare -r p=Publish
export CI="${CI:-true}"
export GITHUB="${GITHUB:-true}" # GITHUB_ACTIONS

# Check commit for skip directives
LP3D_COMMIT_MSG=$(echo $LP3D_COMMIT_MSG | awk '{print toupper($0)}')
if [[ "$(echo $LP3D_COMMIT_MSG | awk '{print toupper($0)}')" == *"SKIP_DEPLOY"* ]]; then
  echo "Skip deploy detected in commit message."
  exit 0
elif [[ "$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')" == *"SKIP_SF_DEPLOY"* ]]; then
  LP3D_SF_DEPLOY_ABORT="true"
else
  unset LP3D_SF_DEPLOY_ABORT
fi

# Check if build is on stale commit
IFS='/' read -ra LP3D_SLUGS <<< "${GITHUB_REPOSITORY}"; unset IFS;
export LP3D_PROJECT_NAME="${LP3D_SLUGS[1]}"
export LP3D_GREP=grep
if [[ "$OSTYPE" == "darwin"* ]]; then export LP3D_GREP=ggrep; fi
[ ! -f "repo.txt" ] && \
curl -s -H "Authorization: Bearer ${GITHUB_TOKEN}" https://api.github.com/repos/${GITHUB_REPOSITORY}/commits/master -o repo.txt
export LP3D_REMOTE=$(${LP3D_GREP} -Po '(?<=: \")(([a-z0-9])\w+)(?=\")' -m 1 repo.txt)
export LP3D_LOCAL=$(git rev-parse HEAD)
if [[ "$LP3D_REMOTE" != "$LP3D_LOCAL" ]]; then
  echo "Build no longer current. Rmote: '$LP3D_REMOTE', Local: '$LP3D_LOCAL' - aborting upload."
  [ -f "repo.txt" ] && echo "Repo response:" && cat repo.txt || :
  exit 0
fi

# Get upload script
wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh

_PRO_FILE_PWD_=${PWD}/mainApp
_EXPORT_CONFIG_ONLY_=1
set +x && echo && source builds/utilities/update-config-files.sh && echo && set -x

# Check commit for version tag
if [[ "$GITHUB_REF" == "refs/tags/"* ]] ; then
  LP3D_GIT_TAG="${GITHUB_REF#refs/tags/}"
  echo "Commit tag $LP3D_GIT_TAG detected.";
  LP3D_DEPLOY_PKG=$(echo "$LP3D_GIT_TAG" | perl -nle 'print "yes" if m{^(?!$)(?:v[0-9]+\.[0-9]+\.[0-9]+_?[^\W]*)?$} || print "no"')
  if [ "$LP3D_DEPLOY_PKG" = "yes" ]; then
    echo "Deploy tag identified."
  fi
fi

# Update release label
if [ -f upload.sh -a -r upload.sh ]; then
  if [ "$LP3D_DEPLOY_PKG" = "yes" ]; then
    LP3D_RELEASE_LABEL="LPub3D ${LP3D_RELEASE_DATE}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
      sed -i "" "s/      RELEASE_TITLE=\"Release build.*/      RELEASE_TITLE=\"${LP3D_RELEASE_LABEL}\"/g" "upload.sh"
    else
      sed -i    "s/      RELEASE_TITLE=\"Release build.*/      RELEASE_TITLE=\"${LP3D_RELEASE_LABEL}\"/" "upload.sh"
    fi
  else
    case ${LP3D_PROJECT_NAME} in
      "lpub3d")
        LP3D_TITLE="Build" ;;
      "lpub3dnext")
        LP3D_TITLE="Next Build" ;;
      *)
        LP3D_TITLE="DevOps Build" ;;
    esac
    LP3D_RELEASE_LABEL="Continuous ${LP3D_TITLE} ${LP3D_VER_BUILD} (r${LP3D_VER_REVISION})"
    if [[ "$OSTYPE" == "darwin"* ]]; then
      sed -i "" "s/      RELEASE_TITLE=\"Continuous build\"/      RELEASE_TITLE=\"${LP3D_RELEASE_LABEL}\"/" "upload.sh"
    else
      sed -i    "s/      RELEASE_TITLE=\"Continuous build\"/      RELEASE_TITLE=\"${LP3D_RELEASE_LABEL}\"/" "upload.sh"
    fi
  fi
  LP3D_RELEASE_DESCRIPTION="LPub3D - An LDraw™ editor for LEGO® style digital building instructions."
  if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i "" "s/  RELEASE_BODY=\"GitHub Actions.*/  RELEASE_BODY=\"${LP3D_RELEASE_DESCRIPTION}\"/g" "upload.sh"
  else
    sed -i    "s/  RELEASE_BODY=\"GitHub Actions.*/  RELEASE_BODY=\"${LP3D_RELEASE_DESCRIPTION}\"/g" "upload.sh"
  fi
else
  echo  "WARNING - Could not update release title and body in upload.sh. File not found."
fi

# Setup GPG sign keys
echo && echo "Initialize default gpg keyring..."
declare -r LP3D_GPGHOME=$(mktemp -d)
declare -r LP3D_OWNRING="${LP3D_GPGHOME}/ownring.auto"
declare -r LP3D_SECRING="${LP3D_GPGHOME}/secring.auto"
declare -r LP3D_PUBRING="${LP3D_GPGHOME}/pubring.auto"
declare -r s=GPGSign

chown -R $(whoami) ${LP3D_GPGHOME}/
chmod 700 ${LP3D_GPGHOME}
cp -f ./builds/utilities/ci/secure/*.auto* ${LP3D_GPGHOME}/
chmod 600 ${LP3D_GPGHOME}/*
export GNUPGHOME=${LP3D_GPGHOME}

(cd ${LP3D_GPGHOME}
gpgconf --kill gpg-agent  # in case agent_genkey fail...
gpg --generate-key --batch <<eoGPGConf
  %echo Started!
  Key-Type: default
  Key-Length: default
  Subkey-Type: default
  Name-Real: Trevor SANDY
  Name-Comment: Yessir
  Name-Email: trevor.sandy@gmail.com
  Expire-Date: 0
  Passphrase: ${GPG_INIT_PASSPHRASE}
  %commit
  %echo Done.
eoGPGConf
) >$s.out 2>&1 && rm $s.out
[ -f $s.out ] && \
echo "WARNING - Failed to initialize default gpg keyring." && tail -80 $s.out || :

# Note: import the owner public key first so the decrypt call can check the private key signature
if [ ! -f $s.out ]; then
  ( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_OWNRING}" ) >$s.out 2>&1 && rm $s.out
  [ -f $s.out ] && \
  echo "WARNING - Could not import public owner keyring into gpg." && tail -80 $s.out || :
fi

if [ ! -f $s.out ]; then
  ( echo ${GPG_SIGN_PASSPHRASE} | gpg --decrypt --pinentry-mode loopback --passphrase-fd 0 \
    --batch --quiet --output "${LP3D_SECRING}" "${LP3D_SECRING}".gpg ) >$s.out 2>&1 && rm $s.out
  [ -f $s.out ] && \
  echo "WARNING - Failed to decrypt secure publish keyring." && tail -80 $s.out || :
fi

if [ ! -f $s.out ]; then
  ( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_SECRING}" ) >$s.out 2>&1 && rm $s.out
  [ -f $s.out ] && \
  echo "WARNING - Could not import secure publish keyring into gpg." && tail -80 $s.out || :
fi

if [ ! -f $s.out ]; then
  ( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_PUBRING}" ) >$s.out 2>&1 && rm $s.out
  [ -f $s.out ] && \
  echo "WARNING - Could not import public publish keyring into gpg." && tail -80 $s.out || :
fi

# Display asset paths info
echo
echo "LP3D_PROJECT_NAME...........${LP3D_PROJECT_NAME}"
[ "${LP3D_DEPLOY_PKG}" = "yes" ] && \
echo "LP3D_RELEASE_TAG............${LP3D_GIT_TAG}" || :
[ -n "${LP3D_RELEASE_LABEL}" ] && \
echo "LP3D_RELEASE_LABEL..........${LP3D_RELEASE_LABEL}" || :
[ -n "${LP3D_RELEASE_DESCRIPTION}" ] && \
echo "LP3D_RELEASE_DESCRIPTION....${LP3D_RELEASE_DESCRIPTION}" || :
echo "LP3D_BUILD_ASSETS PATH......${LP3D_BUILD_ASSETS}"

# Remove artifacts that are not published
rm -f ${LP3D_BUILD_ASSETS}/windows-download/*RunLog.txt > /dev/null 2>&1
rm -f ${LP3D_BUILD_ASSETS}/*/*-debug*.rpm > /dev/null 2>&1

# Publish update assets to Sourceforge in one call - then delete
if [ -d "${LP3D_BUILD_ASSETS}/updates" ]; then
  echo && echo "Publishing update assets..."
  export LP3D_UPDATE_ASSETS=${LP3D_BUILD_ASSETS}/updates
  PublishToSourceforge
fi

# Publish download assets
echo && echo "Publishing download assets..."
LP3D_ASSETS=$(find ${LP3D_BUILD_ASSETS}/ -type f)

# Publish to Github with individual calls
for LP3D_ASSET in ${LP3D_ASSETS}; do
  export LP3D_ASSET_EXT=".${LP3D_ASSET##*.}"
  case ${LP3D_ASSET_EXT} in
    ".exe"|".zip"|".deb"|".rpm"|".zst"|".dmg"|".AppImage"|".html"|".txt")
    LP3D_RELEASE=${LP3D_ASSET}
    PublishToGitHub
    ;;
  esac
done

# Publish to Sourceforge with single call
export LP3D_DOWNLOAD_ASSETS=${LP3D_BUILD_ASSETS}
PublishToSourceforge

exit 0
