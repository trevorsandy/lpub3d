# If there is no version tag in git this one will be used
VERSION = 1.0.0

# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

# Default location of Git directory
exists($$PWD/.git) {
    GIT_DIR=$$PWD/.git
    message(GIT_DIR $$GIT_DIR)
}
# Location of Git directory when building pkg package
exists($$PWD/../upstream/lpub3d/.git) {
    GIT_DIR=$$PWD/../upstream/lpub3d/.git
    message(GIT_DIR $$GIT_DIR)
}
# Location of Git directory when building deb package
exists($$PWD/../../upstream/lpub3d/.git) {
    GIT_DIR=$$PWD/../../upstream/lpub3d/.git
    message(GIT_DIR $$GIT_DIR)
}

# Need to call git with manually specified paths to repository
BASE_GIT_COMMAND = git --git-dir $$shell_quote$$GIT_DIR --work-tree $$shell_quote$$PWD

# Trying to get version from git tag / revision
GIT_VERSION = $$system($$BASE_GIT_COMMAND describe --long 2> $$NULL_DEVICE)

# Check if we only have hash without version number (i.e. not version tag found)
!contains(GIT_VERSION,\d+\.\d+\.\d+) {
    # If there is nothing we simply use version defined manually
    isEmpty(GIT_VERSION) {
        GIT_VERSION = $$VERSION
    } else { # otherwise construct proper git describe string
        GIT_COMMIT_COUNT = $$system($$BASE_GIT_COMMAND rev-list HEAD --count 2> $$NULL_DEVICE)
        isEmpty(GIT_COMMIT_COUNT) {
            GIT_COMMIT_COUNT = 0
        }
        GIT_VERSION = g$$GIT_VERSION-$$GIT_COMMIT_COUNT
    }
}
#message("GIT_VERSION:" $$GIT_VERSION)

# Convert output from gv2.0.20-37-ge99beed-600 into "gv2.0.20.37.ge99beed.600"
GIT_VERSION ~= s/-/"."
GIT_VERSION ~= s/g/""
GIT_VERSION ~= s/v/""

# Separate the build number into major, minor and service pack etc.
VER_MAJOR = $$section(GIT_VERSION, ., 0, 0)
VER_MINOR = $$section(GIT_VERSION, ., 1, 1)
VER_PATCH = $$section(GIT_VERSION, ., 2, 2)
VER_REVISION_STR = $$section(GIT_VERSION, ., 3, 3)
VER_SHA_HASH_STR = $$section(GIT_VERSION, ., 4, 4)
VER_BUILD_STR = $$section(GIT_VERSION, ., 5, 5)
#message("GIT_VERSION PROCESSED:" $$GIT_VERSION)

# Here we process the build date
win32 {
    BUILD_DATE = $$system( date /t )
    BUILD_TIME = $$system( echo %time% )
} else {
    BUILD_DATE = $$system( date "+%d/%m/%Y/%H:%M:%S" )
    BUILD_TIME = $$section(BUILD_DATE, /, 3, 3)
}
# Separate the date into hours, minutes, seconds etc.
DATE_DD = $$section(BUILD_DATE, /, 0, 0)
DATE_MM = $$section(BUILD_DATE, /, 1, 1)
DATE_YY = $$section(BUILD_DATE, /, 2, 2)
#message("BUILD_TIME:" $$BUILD_TIME ) # output the current time

# Adding C preprocessor #DEFINE so we can use it in C++ code
# also here we want full version on every system so using
# _VER_MAJOR, _VER_MINOR, _VER_SVCPK, _BUILD_NUMBER,
# _HASH_SUFFIX, _BUILD_REVISION, _BUILD_DATE, GIT_VERSION
DEFINES += VER_MAJOR=\\\"$$VER_MAJOR\\\"
DEFINES += VER_MINOR=\\\"$$VER_MINOR\\\"
DEFINES += VER_PATCH=\\\"$$VER_PATCH\\\"

DEFINES += BUILD_TIME=\\\"$$BUILD_TIME\\\"
DEFINES += DATE_YY=\\\"$$DATE_YY\\\"
DEFINES += DATE_MM=\\\"$$DATE_MM\\\"
DEFINES += DATE_DD=\\\"$$DATE_DD\\\"

DEFINES += VER_BUILD_STR=\\\"$$VER_BUILD_STR\\\"
DEFINES += VER_SHA_HASH_STR=\\\"$$VER_SHA_HASH_STR\\\"
DEFINES += VER_REVISION_STR=\\\"$$VER_REVISION_STR\\\"
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

# Now we are ready to pass parsed version to Qt
VERSION = $$VER_MAJOR"."$$VER_MINOR"."$$VER_PATCH

# Update the version number file for win/unix during build
VERSION_INFO_WIN = $$PWD/builds/utilities/version_info_win.txt
VERSION_INFO_UNIX = $$PWD/builds/utilities/version_info_posix
COMMAND_WIN = \"$$VER_MAJOR $$VER_MINOR $$VER_PATCH $$VER_REVISION_STR $$VER_BUILD_STR\",\"$$DATE_YY $$DATE_MM $$DATE_DD $$BUILD_TIME\"
COMMAND_UNIX = \"$$VERSION-$$VER_REVISION_STR-$$VER_BUILD_STR-$$VER_SHA_HASH_STR\"
# On Windows generate input file to be consumed by build script
win32 {
   VERSION_INFO_WIN ~= s,/,\\,g
   VERSION_INFO_UNIX ~= s,/,\\,g
   RESULT = $$system(cmd /c echo $$shell_quote$${COMMAND_WIN} > $$shell_quote$${VERSION_INFO_WIN} )
   RESULT = $$system(cmd /c echo $$shell_quote$${COMMAND_UNIX} > $$shell_quote$${VERSION_INFO_UNIX} )
   #message($$quote(cmd /c echo $$shell_quote$${COMMAND_WIN} > $$shell_quote$${VERSION_INFO_WIN}))
   #message($$quote(cmd /c echo $$shell_quote$${COMMAND_UNIX} > $$shell_quote$${VERSION_INFO_UNIX}))

} else { # On linux, update version information in build
    TOUCH_FILE_COMMAND_UNIX = if [ ! -f $$shell_quote($${VERSION_INFO_UNIX}) ]; then touch $$shell_quote($${VERSION_INFO_UNIX}); fi
    TOUCH_FILE_COMMAND_WIN = if [ ! -f $$shell_quote($${VERSION_INFO_WIN}) ]; then touch $$shell_quote($${VERSION_INFO_WIN}); fi
    RESULT = $$system( $$TOUCH_FILE_COMMAND_UNIX )
    RESULT = $$system( $$TOUCH_FILE_COMMAND_WIN )
    RESULT = $$system( echo $$shell_quote$${COMMAND_UNIX} > $$shell_quote($${VERSION_INFO_UNIX}) )
    RESULT = $$system( echo $$shell_quote$${COMMAND_WIN} > $$shell_quote($${VERSION_INFO_WIN}) )

    # On Mac, update Info.plist with executable name which includes version details and full version
    macx {
        INFO_PLIST_FILE = $$PWD/mainApp/Info.plist
        PLIST_COMMAND = /usr/libexec/PlistBuddy -c
        RESULT = $$system( $$PLIST_COMMAND \"Set :CFBundleShortVersionString $${VERSION}\" $$shell_quote($${INFO_PLIST_FILE}) )
        RESULT = $$system( $$PLIST_COMMAND \"Set :CFBundleVersion $${VER_BUILD_STR}\" $$shell_quote($${INFO_PLIST_FILE}) )
        RESULT = $$system( $$PLIST_COMMAND \"Set :com.trevorsandy.lpub3d.GitSHA $${VER_SHA_HASH_STR}\" $$shell_quote($${INFO_PLIST_FILE}) )
    }

}

