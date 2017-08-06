    # These settings are used for package distribution
win32:stage3rdcontent {
    # The Windows package distribution settings below requires a specific dev env configuration.
    # Basically, if you create a projects folder e.g. c:\Users\<user>\Projects and
    # clone the listed GitHub repos under the Projects folder, your dev env will
    # conform to these settings.
    # GitHub clone list:
    # LPub3D           = https://github.com/trevorsandy/lpub3d [Branch: master]
    # 3rdParty Content = https://github.com/trevorsandy/lpub3d_windows_3rdparty [Branch: master]
    #
    # At the end, your Projects folder should look like this:
    # C:\Users\<user>\Projects\
    #                   |
    #                   ---\lpub3d_windows_3rdparty\
    #                   |
    #                   ---\LPub3D\
    #
    # Additionally, when using QtCreator be sure 'Shadow Build' is enabled.
    #
    # source directories
    isEmpty(THIRD_PARTY_SRC):THIRD_PARTY_SRC = $$_PRO_FILE_PWD_/../../lpub3d_windows_3rdparty
    isEmpty(THIRD_PARTY_STG):THIRD_PARTY_STG = $$_PRO_FILE_PWD_/../builds/3rdParty/Windows

    isEmpty(LDGLITE_EXE):LDGLITE_EXE       = $$THIRD_PARTY_SRC/$$VER_LDGLITE/bin/$$QT_ARCH/ldglite.exe
    equals (ARCH, 64) {
        isEmpty(LDVIEW_EXE):LDVIEW_EXE     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/bin/$$QT_ARCH/LDView$$join(ARCH,,,.exe)
        isEmpty(RAYTRACE_EXE):RAYTRACE_EXE = $$THIRD_PARTY_SRC/$$VER_POVRAY_SRC/bin/$$QT_ARCH/lpub3d_trace_cui$$join(ARCH,,,.exe)
    } else {
        isEmpty(LDVIEW_EXE):LDVIEW_EXE     = $$THIRD_PARTY_SRC/$$VER_LDVIEW/bin/$$QT_ARCH/LDView.exe
        isEmpty(RAYTRACE_EXE):RAYTRACE_EXE = $$THIRD_PARTY_SRC/$$VER_POVRAY_SRC/bin/$$QT_ARCH/lpub3d_trace_cui$$join(ARCH,,,.exe)
    }

    include(stage3rdpartycontent.pri)

}
