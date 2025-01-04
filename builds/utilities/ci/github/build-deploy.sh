#!/bin/bash
# Trevor SANDY
# Last Update January 04, 2024
#
# This script is called from .github/workflows/prod_ci_build.yml
#
# Run command:
# bash -ex builds/utilities/ci/github/build-deploy.sh

set +x
echo

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

# GPG sign .sha512 files and publish assets to Github
SignHashAndPublishToGitHub() {
  if [ -f "${LP3D_RELEASE}" ]; then
    case ${LP3D_ASSET_EXT} in
      ".exe"|".zip"|".gz"|".deb"|".rpm"|".zst"|".dmg"|".AppImage")
      if [ -f "${LP3D_RELEASE}.sha512" ]; then
        # Generate hash file signature
        if [ "${LP3D_USE_GPG}" = "true" ]; then
          echo -n "- Signing ${LP3D_RELEASE}.sha512 hash file..."
          ( gpg --home "${LP3D_GPGHOME}" --clearsign -u ${GPG_SIGN_KEY_ALIAS} \
            -o "${LP3D_RELEASE}.sha512.sig" "${LP3D_RELEASE}.sha512" \
          ) >$s.out 2>&1 && rm $s.out
          if [ ! -f $s.out ]; then
            echo "Ok."
            echo -n "- Uploading signature file ${LP3D_RELEASE}.sha512.sig..."
            ( bash upload.sh "${LP3D_RELEASE}.sha512.sig" ) >$p.out 2>&1 && rm $p.out
            [ -f $p.out ] && \
            echo "\nWARNING - ${LP3D_RELEASE}.sha512.sig GitHub upload failed." && tail -80 $p.out || echo "Ok."
          else
            echo "WARNING - Create signature for ${LP3D_RELEASE}.sha512 file failed." && tail -80 $s.out
          fi
        fi
        echo -n "- Uploading hash file ${LP3D_RELEASE}.sha512..."
        ( bash upload.sh "${LP3D_RELEASE}.sha512" ) >$p.out 2>&1 && rm $p.out
        [ -f $p.out ] && \
        echo "\nWARNING - ${LP3D_RELEASE}.sha512 GitHub upload failed." && tail -80 $p.out || echo "Ok."
      else
        echo "WARNING - ${LP3D_RELEASE}.sha512 file not found."
      fi
      ;;
    esac
    echo -n "- Uploading build file ${LP3D_RELEASE}..."
    ( bash upload.sh "${LP3D_RELEASE}" ) >$p.out 2>&1 && rm $p.out
    [ -f $p.out ] && \
    echo "\nERROR - ${LP3D_RELEASE} GitHub upload failed." && tail -80 $p.out && return 3 || echo "Ok."
  else
    echo "ERROR - ${LP3D_RELEASE} file not found."
    return 3
  fi
}

# Set variables
declare -r p=Publish
export CI="${CI:-true}"
export GITHUB="${GITHUB:-true}" # GITHUB_ACTIONS

LP3D_COMMIT_MSG_ORIG="${LP3D_COMMIT_MSG}"
export LP3D_COMMIT_MSG="$(echo ${LP3D_COMMIT_MSG} | awk '{print toupper($0)}')"


# Check commit for skip directives
if [[ "${LP3D_COMMIT_MSG}" == *"SKIP_DEPLOY"* ]]; then
  echo "Skip deploy detected in commit message."
  exit 0
elif [[ "${LP3D_COMMIT_MSG}" == *"SKIP_SF_DEPLOY"* ]]; then
  LP3D_SF_DEPLOY_ABORT=true
else
  unset LP3D_SF_DEPLOY_ABORT
fi

# Check if build is on stale commit
LP3D_REPOSITORY=$GITHUB_REPOSITORY
if [ "$TRAVIS" = "true" ]; then
  LP3D_REPOSITORY=$TRAVIS_REPO_SLUG
elif [ "$APPVEYOR" = "True" ]; then
  LP3D_REPOSITORY=$APPVEYOR_REPO_NAME
fi
oldIFS=$IFS; IFS='/' read -ra LP3D_SLUGS <<< "${LP3D_REPOSITORY}"; IFS=$oldIFS;
export LP3D_PROJECT_NAME="${LP3D_SLUGS[1]}"
curl -s -H "Authorization: Bearer ${GITHUB_TOKEN}" https://api.github.com/repos/${LP3D_REPOSITORY}/commits/master -o repo.txt
export LP3D_REMOTE=$(cat repo.txt | jq -r '.sha')
export LP3D_LOCAL=$(git rev-parse HEAD)
if [[ "$LP3D_REMOTE" != "$LP3D_LOCAL" ]]; then
  echo "WARNING - Build no longer current. Rmote: '$LP3D_REMOTE', Local: '$LP3D_LOCAL' - aborting build deploy."
  [ -f "repo.txt" ] && echo "Repo response:" && cat repo.txt || :
  exit 0
fi

# Get upload script
echo && echo -n "Get GitHub upload script..." && \
( wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh ) >$p.out 2>&1 && rm $p.out
[ -f $p.out ] && \
echo "\nERROR - Could not get GitHub upload script." && tail -80 $p.out && exit 1 || echo "Ok."

_PRO_FILE_PWD_=${PWD}/mainApp
_EXPORT_CONFIG_ONLY_=1
echo && source builds/utilities/update-config-files.sh && echo

# Check commit for version tag
if [[ "$GITHUB_REF" == "refs/tags/"* ]] ; then
  echo "Commit tag $GITHUB_REF_NAME detected." && echo
  LP3D_DEPLOY_PKG=$(echo "$GITHUB_REF_NAME" | perl -nle 'print "yes" if m{^(?!$)(?:v[0-9]+\.[0-9]+\.[0-9]+_?[^\W]*)?$} || print "no"')
  if [ "$LP3D_DEPLOY_PKG" = "yes" ]; then
    echo "Deploy tag $GITHUB_REF_NAME confirmed." && echo
    export LP3D_GIT_TAG=$GITHUB_REF_NAME
  fi
fi

# Update release label
if [ -f upload.sh -a -r upload.sh ]; then
  if [ "$LP3D_DEPLOY_PKG" = "yes" ]; then
    if [ "$TRAVIS" = "true" ]; then
      export LP3D_GIT_TAG="$TRAVIS_TAG"
    elif [ "$APPVEYOR" = "True" ]; then
      export LP3D_GIT_TAG="$APPVEYOR_REPO_TAG_NAME"
    fi
    if [ -n "$LP3D_VERSION" ]; then
       export LP3D_VERSION="$LP3D_VERSION"
    else
       export LP3D_VERSION="${LP3D_GIT_TAG:1}"
    fi
    LP3D_RELEASE_TITLE="LPub3D ${LP3D_RELEASE_DATE}"
    sed -i    "s/      RELEASE_TITLE=\"Release build.*\"/      RELEASE_TITLE=\"${LP3D_RELEASE_TITLE}\"/" "upload.sh"
    LP3D_RELEASE_BODY="LPub3D - An LDraw™ editor for LEGO® style digital building instructions."
    sed -i    "s/  RELEASE_BODY=\"GitHub Actions.*\"/  RELEASE_BODY=\"${LP3D_RELEASE_BODY}\"/" "upload.sh"
  else
    case project-${LP3D_PROJECT_NAME} in
      "project-lpub3d")
        LP3D_BUILD_TYPE="Build" ;;
      "project-lpub3dnext")
        LP3D_BUILD_TYPE="Next Build" ;;
      *)
        LP3D_BUILD_TYPE="DevOps Build" ;;
    esac
    LP3D_RELEASE_TITLE="Continuous ${LP3D_BUILD_TYPE} ${LP3D_VER_BUILD} (${LP3D_VERSION}-r${LP3D_VER_REVISION})"
    sed -i    "s/      RELEASE_TITLE=\"Continuous build\"/      RELEASE_TITLE=\"${LP3D_RELEASE_TITLE}\"/" "upload.sh"
    LP3D_RELEASE_BODY="${LP3D_COMMIT_MSG_ORIG}"
    sed -i    "s/  RELEASE_BODY=\"GitHub Actions.*\"/  RELEASE_BODY=\"${LP3D_RELEASE_BODY}\"/" "upload.sh"
  fi
else
  echo  "WARNING - Could not update release title and body in upload.sh. File not found."
fi

# Setup GPG sign keys
echo "Setup GPG keyring" && echo
declare -r LP3D_GPGHOME=$(mktemp -d)
declare -r LP3D_OWNRING="${LP3D_GPGHOME}/ownring.auto"
declare -r LP3D_SECRING="${LP3D_GPGHOME}/secring.auto"
declare -r LP3D_PUBRING="${LP3D_GPGHOME}/pubring.auto"
declare -r LP3D_SFDEPLOY="${LP3D_GPGHOME}/.sfdeploy"
declare -r s=GPGSign

chown -R $(whoami) ${LP3D_GPGHOME}/
chmod 700 ${LP3D_GPGHOME}
cp -f ./builds/utilities/ci/secure/*.auto* ${LP3D_GPGHOME}/
export LP3D_USE_GPG=true

echo -n "Decoding ${LP3D_SECRING}..." && \
( echo ${GPG_SIGN_KEY_BASE64} | base64 --decode > ${LP3D_SECRING}.gpg ) >$s.out 2>&1 && rm $s.out
[ -f $s.out ] && \
echo "\nWARNING - Could not decode ${LP3D_SECRING}." && LP3D_USE_GPG=false && tail -80 $s.out || echo "Ok."

echo -n "Decoding ${LP3D_SFDEPLOY}..." && \
( echo ${SFDEPLOY_ED25519_BASE64} | base64 --decode > ${LP3D_SFDEPLOY}.gpg ) >$s.out 2>&1 && rm $s.out
[ -f $s.out ] && \
echo "\nWARNING - Could not decode ${LP3D_SFDEPLOY}." && LP3D_SF_DEPLOY_ABORT=true && tail -80 $s.out || echo "Ok."

export GNUPGHOME="${LP3D_GPGHOME}"
export LP3D_HOST_SSH_KEY="${LP3D_SFDEPLOY}_ed25519"

[ "${LP3D_USE_GPG}" = "true" ] && echo -n "Initializing GPG keyring..." && \
( cd ${LP3D_GPGHOME}
gpgconf --kill gpg-agent  # in case agent_genkey fail...
gpg --generate-key --batch <<eoGPGConf
  %echo Started!
  Key-Type: RSA
  Subkey-Type: RSA
  Name-Real: Trevor SANDY
  Name-Comment: Yessir
  Name-Email: trevor.sandy@gmail.com
  Expire-Date: 0
  Passphrase: ${GPG_INIT_PASSPHRASE}
  %commit
  %echo Done.
eoGPGConf
) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "\nWARNING - Failed to initialize GPG keyring." && LP3D_USE_GPG=false && tail -80 $s.out || echo "Ok."

# Import the owner public key first so it can be accessed for the decrypt and signature calls
[ "${LP3D_USE_GPG}" = "true" ] && echo -n "Importing public owner keyring..." && \
( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_OWNRING}" ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "\nWARNING - Could not import public owner keyring into gpg." && LP3D_USE_GPG=false && tail -80 $s.out || echo "Ok."

[ "${LP3D_USE_GPG}" = "true" ] && echo -n "Importing public signing key..." && \
( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_PUBRING}" ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "\nWARNING - Could not import public signing keyring into gpg." && LP3D_USE_GPG=false && tail -80 $s.out  || echo "Ok."

[ "${LP3D_USE_GPG}" = "true" ] && echo -n "Decrypting private signing keyring..." && \
( echo ${GPG_SIGN_PASSPHRASE} | gpg --decrypt --pinentry-mode loopback --passphrase-fd 0 \
  --batch --quiet --output "${LP3D_SECRING}" "${LP3D_SECRING}".gpg ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "\nWARNING - Failed to decrypt private signing keyring." && LP3D_USE_GPG=false && tail -80 $s.out  || echo "Ok."

[ "${LP3D_USE_GPG}" = "true" ] && echo -n "Importing private signing keyring..." && \
( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_SECRING}" ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "\nWARNING - Could not import private signing keyring into gpg." && LP3D_USE_GPG=false && tail -80 $s.out  || echo "Ok."

[ -z "$LP3D_SF_DEPLOY_ABORT" ] && echo -n "Decrypting Sourceforge host ed25519 key..." && \
( echo ${GPG_SIGN_PASSPHRASE} | gpg --decrypt --pinentry-mode loopback --passphrase-fd 0 \
  --batch --quiet --output "${LP3D_HOST_SSH_KEY}" "${LP3D_SFDEPLOY}".gpg ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "\nWARNING - Failed to decrypt Sourceforge host ed25519 key." && LP3D_SF_DEPLOY_ABORT=true && tail -80 $s.out  || echo "Ok."

# Protect private keys access
echo -n "Restricting secret keys permission..." && \
( find ${LP3D_GPGHOME}/ -type f -exec chmod 600 {} \; ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "\nWARNING - Failed to restrict secret keys permission." && LP3D_SF_DEPLOY_ABORT=true && tail -80 $s.out  || echo "Ok."

# Capture the deployment folder if deploy package

# Display asset paths info
echo
echo "LP3D_PROJECT_NAME...........${LP3D_PROJECT_NAME}"
[ -n "${LP3D_VERSION}" ] && \
echo "LP3D_VERSION................${LP3D_VERSION}" || \
echo "LP3D_VERSION................ERROR: NOT SPECIFIED"
[ "${LP3D_DEPLOY_PKG}" = "yes" ] && \
echo "LP3D_RELEASE_TAG............${LP3D_GIT_TAG}" || \
echo "LP3D_RELEASE_TAG............continuous"
[ -n "${LP3D_RELEASE_TITLE}" ] && \
echo "LP3D_RELEASE_TITLE..........${LP3D_RELEASE_TITLE}" || \
echo "LP3D_RELEASE_TITLE..........ERROR: NOT SPECIFIED"
[ -n "${LP3D_RELEASE_BODY}" ] && \
echo "LP3D_RELEASE_BODY...........${LP3D_RELEASE_BODY}" || \
echo "LP3D_RELEASE_BODY...........ERROR: NOT SPECIFIED"
echo "LP3D_BUILD_ASSETS PATH......${LP3D_BUILD_ASSETS}"
echo "GH_ACTION_BUILD_LOG_URL.....${GITHUB_SERVER_URL}/${GITHUB_REPOSITORY}/actions/runs/${GITHUB_RUN_ID}"

[ "${LP3D_USE_GPG}" = "false" ] && \
echo "WARNING - GPG is not available. Sign sha512 hash will be skipped."

# Remove download artifacts that should not be published
declare -r c=Clean
[ -d "${LP3D_BUILD_ASSETS}" ] && \
echo && echo -n "Remove download artifacts that should not be published..." && \
( find "${LP3D_BUILD_ASSETS}"/*-download -type f \( \
  -name 'Dockerfile' -o \
  -name '*_Assets.zip' -o \
  -name '*_assets.tar.gz' -o \
  -name '*.debian.tar.xz' -o \
  -name '*-debug*.rpm' -o \
  -name '*.buildinfo' -o \
  -name '*.changes' -o \
  -name '*.dmp' -o \
  -name '*.dsc' -o \
  -name '*.log' -o \
  -name '*.sh' \) \
  -exec rm -rf {} \;
) >$c.out 2>&1 && rm $c.out || :
[ -f $c.out ] && \
echo "\nWARNING - Failed to clean up artifacts." && tail -80 $c.out || echo "Ok."

# Publish assets
echo && echo "Publishing Github download assets..." && \
export LP3D_UPDATE_ASSETS="${LP3D_BUILD_ASSETS}/updates"
export LP3D_DOWNLOAD_ASSETS="${LP3D_BUILD_ASSETS}/downloads"
[ ! -d "${LP3D_DOWNLOAD_ASSETS}" ] && mkdir -p ${LP3D_DOWNLOAD_ASSETS} || :

# Generate hash signature files and publish Github download assets
for LP3D_ASSET in $(find "${LP3D_BUILD_ASSETS}"/*-download -type f); do
  export LP3D_ASSET_EXT=".${LP3D_ASSET##*.}"
  case ${LP3D_ASSET_EXT} in
    ".exe"|".zip"|".gz"|".deb"|".rpm"|".zst"|".dmg"|".AppImage"|".html"|".txt")
    # Move assets to downloads folder
    export LP3D_RELEASE="${LP3D_DOWNLOAD_ASSETS}/$(basename ${LP3D_ASSET})"
    mv ${LP3D_ASSET} ${LP3D_DOWNLOAD_ASSETS}
    [ -f "${LP3D_ASSET}.sha512" ] && \
    mv ${LP3D_ASSET}.sha512 ${LP3D_DOWNLOAD_ASSETS} || :

    SignHashAndPublishToGitHub
    ;;
  esac
done

# Publish update and download assets to Sourceforge
if [ -z "$LP3D_SF_DEPLOY_ABORT" ]; then
  export LP3D_ASSET_EXT=".all"
  export LP3D_DEPLOY_PKG="$LP3D_DEPLOY_PKG"
  echo && echo -n "Publishing Sourceforge update and download assets..."
  ( chmod a+x builds/utilities/ci/sfdeploy.sh && ./builds/utilities/ci/sfdeploy.sh ) >$p.out 2>&1 && mv $p.out $p.ok
  [ -f $p.out ] && \
  echo "\nERROR - Publish Sourceforge assets failed." && tail -80 $p.out && exit 1 || echo "Ok." && cat $p.ok
fi

exit 0
