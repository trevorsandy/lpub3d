#!/bin/bash
# Trevor SANDY
# Last Update May 25, 2022
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
    [ -f $p.out ] && echo "WARNING - Sourceforge upload failed." && tail -80 $p.out || cat $p.ok
    if [ -n "${LP3D_UPDATE_ASSETS}" ]; then
      rm -rf ${LP3D_UPDATE_ASSETS}
      unset LP3D_UPDATE_ASSETS
    fi
  fi
}

# GPG sign .sha512 files and publish assets to Github
SignHashAndPublishToGitHub() {
  if [ -f "${LP3D_RELEASE}" ]; then
    case ${LP3D_ASSET_EXT} in
      ".exe"|".zip"|".deb"|".rpm"|".zst"|".dmg"|".AppImage")
      if [ -f "${LP3D_RELEASE}.sha512" ]; then
        # Generate hash file signature
        if [[ "${LP3D_USE_GPG}" = "true" && -f "${LP3D_ASSET}.sha512" ]]; then
          echo "- Signing ${LP3D_ASSET}.sha512 file..."
          ( gpg --home "${LP3D_GPGHOME}" --clearsign -u ${GPG_SIGN_KEY_ALIAS} \
            -o "${LP3D_RELEASE}.sha512.sig" "${LP3D_RELEASE}.sha512" \
          ) >$s.out 2>&1 && rm $s.out
          if [ ! -f $s.out ]; then
            echo "- Uploading signature file ${LP3D_ASSET}.sha512.sig..."
            ( bash upload.sh "${LP3D_RELEASE}.sha512.sig" ) >$p.out 2>&1 && rm $p.out
            [ -f $p.out ] && \
            echo "WARNING - ${LP3D_RELEASE}.sha512.sig GitHub upload failed." && tail -80 $p.out || echo "Ok."
          else
            echo "WARNING - Create signature file failed." && tail -80 $s.out
          fi
        fi
        echo "- Uploading hash file ${LP3D_ASSET}.sha512..."
        ( bash upload.sh "${LP3D_RELEASE}.sha512" ) >$p.out 2>&1 && rm $p.out
        [ -f $p.out ] && \
        echo "WARNING - ${LP3D_RELEASE}.sha512 GitHub upload failed." && tail -80 $p.out || echo "Ok."
      else
        echo "WARNING - ${LP3D_RELEASE}.sha512 file not found."
      fi
      ;;
    esac
    echo "- Uploading build file ${LP3D_ASSET}..."
    ( bash upload.sh "${LP3D_RELEASE}" ) >$p.out 2>&1 && rm $p.out
    [ -f $p.out ] && \
    echo "WARNING - ${LP3D_RELEASE} GitHub upload failed." && tail -80 $p.out || echo "Ok."
  else
    echo "WARNING - ${LP3D_RELEASE} file not found."
  fi
}

# Set variables
declare -r p=Publish
export CI="${CI:-true}"
export GITHUB="${GITHUB:-true}" # GITHUB_ACTIONS
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
  echo "Commit tag $GITHUB_REF_NAME detected.";
  LP3D_DEPLOY_PKG=$(echo "$GITHUB_REF_NAME" | perl -nle 'print "yes" if m{^(?!$)(?:v[0-9]+\.[0-9]+\.[0-9]+_?[^\W]*)?$} || print "no"')
  if [ "$LP3D_DEPLOY_PKG" = "yes" ]; then
    echo "Deploy tag $GITHUB_REF_NAME confirmed."
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
    LP3D_RELEASE_DESCRIPTION="LPub3D - An LDraw™ editor for LEGO® style digital building instructions."
    if [[ "$OSTYPE" == "darwin"* ]]; then
      sed -i "" "s/  RELEASE_BODY=\"GitHub Actions.*/  RELEASE_BODY=\"${LP3D_RELEASE_DESCRIPTION}\"/g" "upload.sh"
    else
      sed -i    "s/  RELEASE_BODY=\"GitHub Actions.*/  RELEASE_BODY=\"${LP3D_RELEASE_DESCRIPTION}\"/g" "upload.sh"
    fi
  else
    case project-${LP3D_PROJECT_NAME} in
      "project-lpub3d")
        LP3D_TITLE="Build" ;;
      "project-lpub3dnext")
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
    LP3D_RELEASE_DESCRIPTION="${LP3D_COMMIT_MSG}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
      sed -i "" "s/  RELEASE_BODY=\"GitHub Actions.*/  RELEASE_BODY=\"${LP3D_RELEASE_DESCRIPTION}\"/g" "upload.sh"
    else
      sed -i    "s/  RELEASE_BODY=\"GitHub Actions.*/  RELEASE_BODY=\"${LP3D_RELEASE_DESCRIPTION}\"/g" "upload.sh"
    fi
  fi
else
  echo  "WARNING - Could not update release title and body in upload.sh. File not found."
fi

# Setup GPG sign keys
echo && echo "Initialize default GPG keyring..."
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

echo "Decoding ${LP3D_SECRING}..." && \
( echo ${GPG_SIGN_KEY_BASE64} | base64 --decode > ${LP3D_SECRING}.gpg ) >$s.out 2>&1 && rm $s.out
[ -f $s.out ] && \
echo "WARNING - Could not decode ${LP3D_SECRING}." && LP3D_USE_GPG=false && tail -80 $s.out || echo "Ok."

echo "Decoding ${LP3D_SFDEPLOY}..." && \
( echo ${SFDEPLOY_RSA_BASE64} | base64 --decode > ${LP3D_SFDEPLOY}.gpg ) >$s.out 2>&1 && rm $s.out
[ -f $s.out ] && \
echo "WARNING - Could not decode ${LP3D_SFDEPLOY}." && LP3D_SF_DEPLOY_ABORT=true && tail -80 $s.out || echo "Ok."

export GNUPGHOME="${LP3D_GPGHOME}"
export LP3D_HOST_RSA_KEY="${LP3D_SFDEPLOY}"_rsa

[ "${LP3D_USE_GPG}" = "true" ] && echo "Initializing default GPG keyring..." && \
( cd ${LP3D_GPGHOME}
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
) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "WARNING - Failed to initialize default GPG keyring." && LP3D_USE_GPG=false && tail -80 $s.out || echo "Ok."

# Import the owner public key first so it can be accessed for the decrypt and signature calls
[ "${LP3D_USE_GPG}" = "true" ] && echo "Importing public owner keyring..." && \
( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_OWNRING}" ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "WARNING - Could not import public owner keyring into gpg." && LP3D_USE_GPG=false && tail -80 $s.out || echo "Ok."

[ "${LP3D_USE_GPG}" = "true" ] && echo "Importing public signing key..." && \
( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_PUBRING}" ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "WARNING - Could not import public signing keyring into gpg." && LP3D_USE_GPG=false && tail -80 $s.out  || echo "Ok."

[ "${LP3D_USE_GPG}" = "true" ] && echo "Decrypting private signing keyring..." && \
( echo ${GPG_SIGN_PASSPHRASE} | gpg --decrypt --pinentry-mode loopback --passphrase-fd 0 \
  --batch --quiet --output "${LP3D_SECRING}" "${LP3D_SECRING}".gpg ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "WARNING - Failed to decrypt private signing keyring." && LP3D_USE_GPG=false && tail -80 $s.out  || echo "Ok."

[ "${LP3D_USE_GPG}" = "true" ] && echo "Importing private signing keyring..." && \
( gpg --home "${LP3D_GPGHOME}" --import "${LP3D_SECRING}" ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "WARNING - Could not import private signing keyring into gpg." && LP3D_USE_GPG=false && tail -80 $s.out  || echo "Ok."

[ -z "$LP3D_SF_DEPLOY_ABORT" ] && echo "Decrypting Sourceforge host rsa key..." && \
( echo ${GPG_SIGN_PASSPHRASE} | gpg --decrypt --pinentry-mode loopback --passphrase-fd 0 \
  --batch --quiet --output "${LP3D_HOST_RSA_KEY}" "${LP3D_SFDEPLOY}".gpg ) >$s.out 2>&1 && rm $s.out || :
[ -f $s.out ] && \
echo "WARNING - Failed to decrypt Sourceforge host rsa key." && LP3D_SF_DEPLOY_ABORT=true && tail -80 $s.out  || echo "Ok."

echo "Restricting secret keys permission..." && \
find ${LP3D_GPGHOME}/ -type f -exec chmod 600 {} \;

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

# Publish update assets to Sourceforge in one call - then delete
if [ -d "${LP3D_BUILD_ASSETS}/updates" ]; then
  echo && echo "Publishing update assets..."
  export LP3D_UPDATE_ASSETS=${LP3D_BUILD_ASSETS}/updates
  PublishToSourceforge
fi

# Remove artifacts that should not be published
declare -r c=Clean
[ -d "${LP3D_BUILD_ASSETS}" ] && \
echo && echo "Clean up artifacts that should not be published..." && \
( find ${LP3D_BUILD_ASSETS}/ -type f \( \
  -name 'Dockerfile' -o \
  -name '*.debian.tar.xz' -o \
  -name '*-debug*.rpm' -o \
  -name '*.buildinfo' -o \
  -name '*.changes' -o \
  -name '*.dsc' -o \
  -name '*.log' -o \
  -name '*.sh' \) \
  -exec rm -rf {} \;
) >$c.out 2>&1 && rm $c.out || :
[ -f $c.out ] && \
echo "WARNING - Failed to clean up artifacts." && tail -80 $c.out || echo "Ok."

# Publish download assets
echo && echo "Publishing download assets..." && \
LP3D_ASSETS=$(find ${LP3D_BUILD_ASSETS}/ -type f)

# Sign hash files and publish download assets to Github
for LP3D_ASSET in ${LP3D_ASSETS}; do
  export LP3D_ASSET_EXT=".${LP3D_ASSET##*.}"
  # Process individual release asset
  case ${LP3D_ASSET_EXT} in
    ".exe"|".pdb"|".zip"|".deb"|".rpm"|".zst"|".dmg"|".AppImage"|".html"|".txt")
    if [ "${LP3D_ASSET_EXT}" = ".pdb" ]; then
      LP3D_RELEASE=$(echo ${LP3D_ASSET/.pdb/-${LP3D_APP_VERSION_LONG}.pdb})
      ( mv -v "${LP3D_ASSET}" "${LP3D_RELEASE}" ) >$c.out 2>&1 && rm $c.out
      [ -f $c.out ] && echo "WARNING - Rename ${LP3D_ASSET} failed." && tail -80 $c.out || echo "Ok."
    else
      LP3D_RELEASE=${LP3D_ASSET}
    fi
    SignHashAndPublishToGitHub
    ;;
  esac
done

# Publish to Sourceforge with single call
export LP3D_DOWNLOAD_ASSETS=${LP3D_BUILD_ASSETS}
PublishToSourceforge

exit 0
