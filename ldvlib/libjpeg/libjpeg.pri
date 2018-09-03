INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
      $$PWD/jpeglib.h \
      $$PWD/jerror.h \
      $$PWD/jmorecfg.h \
      $$PWD/jconfig.h \
      $$PWD/cderror.h \
      $$PWD/cdjpeg.h \
      $$PWD/jdct.h \
      $$PWD/jinclude.h \
      $$PWD/jmemsys.h \
      $$PWD/jpegint.h \
      $$PWD/jversion.h \
      $$PWD/transupp.h

SOURCES += \
      $$PWD/jaricom.c \
      $$PWD/jcapimin.c \
      $$PWD/jcapistd.c \
      $$PWD/jcarith.c \
      $$PWD/jccoefct.c \
      $$PWD/jccolor.c \
      $$PWD/jcdctmgr.c \
      $$PWD/jchuff.c \
      $$PWD/jcinit.c \
      $$PWD/jcmainct.c \
      $$PWD/jcmarker.c \
      $$PWD/jcmaster.c \
      $$PWD/jcomapi.c \
      $$PWD/jcparam.c \
      $$PWD/jcprepct.c \
      $$PWD/jcsample.c \
      $$PWD/jctrans.c \
      $$PWD/jdapimin.c \
      $$PWD/jdapistd.c \
      $$PWD/jdarith.c \
      $$PWD/jdatadst.c \
      $$PWD/jdatasrc.c \
      $$PWD/jdcoefct.c \
      $$PWD/jdcolor.c \
      $$PWD/jddctmgr.c \
      $$PWD/jdhuff.c \
      $$PWD/jdinput.c \
      $$PWD/jdmainct.c \
      $$PWD/jdmarker.c \
      $$PWD/jdmaster.c \
      $$PWD/jdmerge.c \
      $$PWD/jdpostct.c \
      $$PWD/jdsample.c \
      $$PWD/jdtrans.c \
      $$PWD/jerror.c \
      $$PWD/jfdctflt.c \
      $$PWD/jfdctfst.c \
      $$PWD/jfdctint.c \
      $$PWD/jidctflt.c \
      $$PWD/jidctfst.c \
      $$PWD/jidctint.c \
      $$PWD/jquant1.c \
      $$PWD/jquant2.c \
      $$PWD/jutils.c \
      $$PWD/jmemmgr.c

# memory manager
SOURCES += \
      $$PWD/jmemnobs.c

#   Selecting a memory manager
# --------------------------

# The IJG code is capable of working on images that are too big to fit in main
# memory; data is swapped out to temporary files as necessary.  However, the
# code to do this is rather system-dependent.  We provide five different
# memory managers:

# * jmemansi.c  This version uses the ANSI-standard library routine tmpfile(),
#     which not all non-ANSI systems have.  On some systems
#     tmpfile() may put the temporary file in a non-optimal
#     location; if you don't like what it does, use jmemname.c.

# * jmemname.c  This version creates named temporary files.  For anything
#     except a Unix machine, you'll need to configure the
#     select_file_name() routine appropriately; see the comments
#     near the head of jmemname.c.  If you use this version, define
#     NEED_SIGNAL_CATCHER in jconfig.h to make sure the temp files
#     are removed if the program is aborted.

# * jmemnobs.c  (That stands for No Backing Store :-).)  This will compile on
#     almost any system, but it assumes you have enough main memory
#     or virtual memory to hold the biggest images you work with.

# * jmemdos.c This should be used with most 16-bit MS-DOS compilers.
#     See the system-specific notes about MS-DOS for more info.
#     IMPORTANT: if you use this, define USE_MSDOS_MEMMGR in
#     jconfig.h, and include the assembly file jmemdosa.asm in the
#     programs.  The supplied makefiles and jconfig files for
#     16-bit MS-DOS compilers already do both.

# * jmemmac.c Custom version for Apple Macintosh; see the system-specific
#     notes for Macintosh for more info.

# If you have plenty of (real or virtual) main memory, just use jmemnobs.c.
# "Plenty" means about ten bytes for every pixel in the largest images
# you plan to process, so a lot of systems don't meet this criterion.
# If yours doesn't, try jmemansi.c first.  If that doesn't compile, you'll have
# to use jmemname.c; be sure to adjust select_file_name() for local conditions.
# You may also need to change unlink() to remove() in close_backing_store().

# Except with jmemnobs.c or jmemmac.c, you need to adjust the DEFAULT_MAX_MEM
# setting to a reasonable value for your system (either by adding a #define for
# DEFAULT_MAX_MEM to jconfig.h, or by adding a -D switch to the Makefile).
# This value limits the amount of data space the program will attempt to
# allocate.  Code and static data space isn't counted, so the actual memory
# needs for cjpeg or djpeg are typically 100 to 150Kb more than the max-memory
# setting.  Larger max-memory settings reduce the amount of I/O needed to
# process a large image, but too large a value can result in "insufficient
# memory" failures.  On most Unix machines (and other systems with virtual
# memory), just set DEFAULT_MAX_MEM to several million and forget it.  At the
# other end of the spectrum, for MS-DOS machines you probably can't go much
# above 300K to 400K.  (On MS-DOS the value refers to conventional memory only.
# Extended/expanded memory is handled separately by jmemdos.c.)

