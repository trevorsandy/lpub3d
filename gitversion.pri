# If there is no version tag in git this one will be used
VERSION = 1.0.0

# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

GIT_DIR = undefined
# Default location of Git directory
exists($$PWD/.git) {
    GIT_DIR = $$PWD/.git
}

# enable to Test
# GIT_DIR = undefined
equals(GIT_DIR, undefined) {
    open_build_service = $$(OBS)
    contains(open_build_service, true) {
        message("~~~ GIT_DIR [OBS, USING VERSION_INFO FILE] $$GIT_VER_FILE ~~~")
    } else {
        message("~~~ GIT_DIR [UNDEFINED, USING VERSION_INFO FILE] $$GIT_VER_FILE ~~~")
    }
    GIT_VER_FILE = $$system_path($$PWD/builds/utilities/version.info)
    exists($$GIT_VER_FILE) {
        GIT_VERSION = $$cat($$GIT_VER_FILE, lines)
    } else {
        message("~~~ ERROR! VERSION_INFO FILE $$GIT_VER_FILE NOT FOUND ~~~")
        UNDEFINED_VERSION = $${VERSION}.0.0.00000000.noversion
        message("~~~ GIT_DIR [UNDEFINED, USING VERSION] $$UNDEFINED_VERSION ~~~")
        GIT_VERSION  = $$UNDEFINED_VERSION
        GIT_VERSION ~= s/\./" "
    }
    # Separate the build number into major, minor and service pack etc...
    VER_MAJOR        = $$section(GIT_VERSION, " ", 0, 0)
    VER_MINOR        = $$section(GIT_VERSION, " ", 1, 1)
    VER_PATCH        = $$section(GIT_VERSION, " ", 2, 2)
    VER_REVISION_STR = $$section(GIT_VERSION, " ", 3, 3)
    VER_BUILD_STR    = $$section(GIT_VERSION, " ", 4, 4)
    VER_SHA_HASH_STR = $$section(GIT_VERSION, " ", 5, 5)
    VER_SUFFIX       = $$section(GIT_VERSION, " ", 6, 6)
} else {
    # Need to call git with manually specified paths to repository
    BASE_GIT_COMMAND = git --git-dir $$shell_quote$$GIT_DIR --work-tree $$shell_quote$$PWD

    # Trying to get version from git tag / revision
    GIT_VERSION = $$system($$BASE_GIT_COMMAND describe --tags --match v* --long 2> $$NULL_DEVICE)

    # Check if we only have hash without version number (i.e. not version tag found)
    !contains(GIT_VERSION,\d+\.\d+\.\d+) {
        # If there is nothing we simply use version defined manually
        isEmpty(GIT_VERSION) {
            GIT_VERSION = $${VERSION}-00-00000000-000
            message("~~~ ERROR! GIT_VERSION NOT DEFINED, USING $$GIT_VERSION ~~~")
        } else { # otherwise construct proper git describe string
            GIT_COMMIT_COUNT = $$system($$BASE_GIT_COMMAND rev-list HEAD --count 2> $$NULL_DEVICE)
            isEmpty(GIT_COMMIT_COUNT) {
                GIT_COMMIT_COUNT = 0
                message("~~~ ERROR! GIT_COMMIT_COUNT NOT DEFINED, USING $$GIT_COMMIT_COUNT ~~~")
            }
            GIT_VERSION = g$$GIT_VERSION-$$GIT_COMMIT_COUNT
        }
    }
    # Token position       0 1 2  3  4   5      [6]
    # Version string       2 0 20 17 663 410fdd7 beta1
    #message("~~~ DEBUG ~~ GIT_VERSION [RAW]: $$GIT_VERSION")

    # Convert output from gv2.0.20-37-ge99beed-600 into "gv2.0.20.37.ge99beed.600"
    GIT_VERSION ~= s/-/"."
    GIT_VERSION ~= s/gv/""
    #message(~~~ DEBUG ~~ GIT_VERSION [FORMATTED]: $$GIT_VERSION)

    # Separate the build number into major, minor and service pack etc.
    VER_MAJOR        = $$section(GIT_VERSION, ., 0, 0)
    VER_MINOR        = $$section(GIT_VERSION, ., 1, 1)
    VER_REVISION_STR = $$section(GIT_VERSION, ., 3, 3)
    VER_SHA_HASH_STR = $$section(GIT_VERSION, ., 4, 4)
    VER_BUILD_STR    = $$section(GIT_VERSION, ., 5, 5)
    # Capture and convert version suffix - everything after "_" if it exist
    GIT_VER_TMP      = $$section(GIT_VERSION, ., 2, 2)
    VER_SUFFIX       = $$section(GIT_VER_TMP, _, 1)
    !isEmpty(VER_SUFFIX) {
        VER_PATCH    = $$section(GIT_VER_TMP, _, 0, 0)
    } else {
        VER_PATCH    = $$section(GIT_VERSION, ., 2, 2)
    }

    # Strip leading 'g' from sha hash
    VER_SHA_HASH_STR ~= s/g/""

    # Get the git repository name
    GIT_BASE_NAME = $$system($$BASE_GIT_COMMAND rev-parse --show-toplevel 2> $$NULL_DEVICE)
    GIT_BASE_NAME = $$basename(GIT_BASE_NAME)
}

# Here we process the build date and time
win32 {
    BUILD_DATE = $$system( date /t )
    BUILD_TIME = $$system( echo %time% )
} else {
    BUILD_DATE = $$system( date "+%d/%m/%Y/%H:%M:%S" )
    BUILD_TIME = $$section(BUILD_DATE, /, 3, 3)
}
#message(~~~ DEBUG ~~ BUILD_DATE: $$BUILD_DATE) # output the current date
#message(~~~ DEBUG ~~ BUILD_TIME: $$BUILD_TIME) # output the current time

# Separate the date into day month, year
appveyor_ci {
    # AppVeyor CI uses date format 'Day MM/DD/YY'
    message("~~~ USING APPVEYOR WINDOWS DATE FORMAT ~~~")
    BUILD_DATE ~= s/[\sA-Za-z\s]/""
    DATE_MM = $$section(BUILD_DATE, /, 0, 0)
    DATE_DD = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
} else {
    message("~~~ BUILD DATE $$BUILD_DATE ~~~")
    DATE_DD = $$section(BUILD_DATE, /, 0, 0)
    DATE_DD = $$last(DATE_DD)
    DATE_MM = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
}

# Resolve Continuous Build version number
TRAVIS_BUILD   = $$(TRAVIS_TAG)
APPVEYOR_BUILD = $$(APPVEYOR_REPO_TAG_NAME)
LOCAL_BUILD    = $$system($$BASE_GIT_COMMAND describe --tags --abbrev=0 2> $$NULL_DEVICE)
contains(TRAVIS_BUILD,continuous) {
    BUILD_TYPE = $$TRAVIS_BUILD
} else:contains(APPVEYOR_BUILD,continuous) {
    BUILD_TYPE = $$APPVEYOR_BUILD
} else:contains(LOCAL_BUILD,continuous) {
    BUILD_TYPE = $$LOCAL_BUILD
}
contains(BUILD_TYPE,continuous) {
    DEFINES += LP3D_CONTINUOUS_BUILD
}

!equals(GIT_BASE_NAME, $$lower("lpub3d")) {
    equals(GIT_BASE_NAME, $$lower("lpub3dnext")) {
        DEFINES += LP3D_NEXT_BUILD
    } else {
        DEFINES += LP3D_DEVOPS_BUILD
    }
}

# C preprocessor #DEFINE to use in C++ code
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

LP3D_VERSION_INFO = $$VER_MAJOR $$VER_MINOR $$VER_PATCH $$VER_REVISION_STR $$VER_BUILD_STR $$VER_SHA_HASH_STR
!isEmpty(VER_SUFFIX) {
    DEFINES += VER_SUFFIX=\\\"$$VER_SUFFIX\\\"
    LP3D_VERSION_INFO += $$VER_SUFFIX
}

# Now we are ready to pass parsed version to Qt
VERSION = $$VER_MAJOR"."$$VER_MINOR"."$$VER_PATCH"."$$VER_REVISION_STR

# Display the complete version string
message(~~~ LP3D_VERSION_INFO: $$LP3D_VERSION_INFO $$upper($$BUILD_TYPE) ~~~)
