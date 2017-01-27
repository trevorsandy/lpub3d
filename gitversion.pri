# If there is no version tag in git this one will be used
VERSION = 1.0.0

# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

# Need to call git with manually specified paths to repository
BASE_GIT_COMMAND = git --git-dir $$shell_quote$$PWD/.git --work-tree $$shell_quote$$PWD

# Trying to get version from git tag / revision
GIT_VERSION = $$system($$BASE_GIT_COMMAND describe --long 2> $$NULL_DEVICE)

# Check if we only have hash without version number
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

# Turns describe output like 0.1.5-42-g652c397 into "0.1.5.42.652c397"
GIT_VERSION ~= s/-/"."
GIT_VERSION ~= s/g/""
GIT_VERSION ~= s/v/""

# Separate the build number into major, minor and service pack etc.
VER_MAJOR = $$section(GIT_VERSION, ., 0, 0)
VER_MINOR = $$section(GIT_VERSION, ., 1, 1)
VER_SP = $$section(GIT_VERSION, ., 2, 2)
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

# Now we are ready to pass parsed version to Qt
VERSION = $$GIT_VERSION
win32 { # On windows version can only be numerical so remove commit hash
   VERSION ~= s/\.\d+\.[a-f0-9]{6,}//
}
#message("VERSION PROCESSED (win32):" $$VERSION)

# Adding C preprocessor #DEFINE so we can use it in C++ code
# also here we want full version on every system so using
# _VER_MAJOR, _VER_MINOR, _VER_SVCPK, _BUILD_NUMBER,
# _HASH_SUFFIX, _BUILD_REVISION, _BUILD_DATE, GIT_VERSION
DEFINES += VER_MAJOR=\\\"$$VER_MAJOR\\\"
DEFINES += VER_MINOR=\\\"$$VER_MINOR\\\"
DEFINES += VER_SP=\\\"$$VER_SP\\\"

DEFINES += BUILD_TIME=\\\"$$BUILD_TIME\\\"
DEFINES += DATE_YY=\\\"$$DATE_YY\\\"
DEFINES += DATE_MM=\\\"$$DATE_MM\\\"
DEFINES += DATE_DD=\\\"$$DATE_DD\\\"

DEFINES += VER_BUILD_STR=\\\"$$VER_BUILD_STR\\\"
DEFINES += VER_SHA_HASH_STR=\\\"$$VER_SHA_HASH_STR\\\"
DEFINES += VER_REVISION_STR=\\\"$$VER_REVISION_STR\\\"
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

# By default Qt only uses major and minor version for Info.plist on Mac.
# This will rewrite Info.plist with full version
#macx {
#    INFO_PLIST_PATH = $$shell_quote($${OUT_PWD}/$${TARGET}.app/Contents/Info.plist)
#    QMAKE_POST_LINK += /usr/libexec/PlistBuddy -c \"Set :CFBundleGetInfoString $${VERSION}\" $${INFO_PLIST_PATH}
#}

win32 {
   BUILD_INFO = $$PWD/builds/windows/setup/versioninfo.txt
   BUILD_INFO ~= s,/,\\,g
   COMMAND = \"$$VER_MAJOR $$VER_MINOR $$VER_SP $$VER_REVISION_STR $$VER_BUILD_STR\",\"$$DATE_YY $$DATE_MM $$DATE_DD $$BUILD_TIME\"
   QMAKE_POST_LINK += $$quote(cmd /c echo $$shell_quote$${COMMAND} > $$shell_quote$${BUILD_INFO})
   #message($$quote(cmd /c echo $$shell_quote$${COMMAND} > $$shell_quote$${BUILD_INFO}))
   message($${COMMAND})
}

