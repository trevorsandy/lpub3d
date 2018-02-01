unix:!macx {
  # Install libraries not available natively - used for RHEL builds
  install_qt_libs       = $$(get_qt5)
  install_local_el_libs = $$(get_local_libs)

  isEmpty(INSTALL_SYSCONF):INSTALL_SYSCONF             = /etc
  isEmpty(LP3D_LIBDIR):LP3D_LIBDIR                     = $${INSTALL_PREFIX}/lib$${LIB_ARCH}/lpub3dlib
  isEmpty(LP3D_LIBDIR_LLVM):LP3D_LIBDIR_LLVM           = $${LP3D_LIBDIR}/llvm
  isEmpty(LP3D_LIBDIR_PKGCONFIG):LP3D_LIBDIR_PKGCONFIG = $${LP3D_LIBDIR}/pkgconfig
  isEmpty(LP3D_LIBDIR_QT5):LP3D_LIBDIR_QT5             = $${LP3D_LIBDIR}/qt5
  isEmpty(LP3D_SO_CONF_DIR):LP3D_SO_CONF_DIR           = $${INSTALL_SYSCONF}/ld.so.conf.d
  isEmpty(LP3D_DRM_RULES):LP3D_DRM_RULES               = $${INSTALL_PREFIX}/lib/udev/rules.d
  isEmpty(LP3D_LOCAL_LIBDIR_USR):LP3D_LOCAL_LIBDIR_USR = $$system("cd $$_PRO_FILE_PWD_/../../ && echo $PWD/usr")
  isEmpty(LP3D_LOCAL_LIBDIR_ETC):LP3D_LOCAL_LIBDIR_ETC = $$system("cd $$_PRO_FILE_PWD_/../../ && echo $PWD/etc")

  equals(install_qt_libs, 1) {
    message("~~~ INSTALL QT $$QT_VERSION LIBS AND PLUGINS SPECIFIED ~~~")

    LP3D_QTCONF_FILE     = $$_PRO_FILE_PWD_/qt.conf
    LP3D_QTCONF_LINES   += [Paths]
    LP3D_QTCONF_LINES   += Prefix=$$LP3D_LIBDIR_QT5
    LP3D_QTCONF_LINES   += Libraries=lib
    LP3D_QTCONF_LINES   += Plugins=plugins

    !write_file($$LP3D_QTCONF_FILE, LP3D_QTCONF_LINES ) {
      message("~~~ ERROR - Could not create $$LP3D_QTCONF_FILE ~~~")
    } else {
      message("~~~ FILE $$LP3D_QTCONF_FILE CREATED ~~~")
    }

    LP3D_QTLDCONF_FILE   = $$_PRO_FILE_PWD_/lpub3d-qtlibs.conf
    LP3D_QTLDCONF_LINES += $$LP3D_LIBDIR_QT5

    !write_file($$LP3D_QTLDCONF_FILE, LP3D_QTLDCONF_LINES) {
      message("~~~ ERROR - Could not create $$LP3D_QTLDCONF_FILE ~~~")
    } else {
      message("~~~ FILE $$LP3D_QTLDCONF_FILE CREATED ~~~")
    }

    LP3D_QTQRC_FILE     = $$_PRO_FILE_PWD_/lpub3d.qrc
    exists($$LP3D_QTQRC_FILE) {
      system("sed -i '/<\/qresource>/a <qresource prefix=\"\/qt\/etc\"><file alias=\"qt.conf\">qt.conf<\/file><\/qresource>' $$LP3D_QTQRC_FILE >/dev/null")
      message("~~~ FILE $$LP3D_QTQRC_FILE UDATED ~~~")
    } else {
      message("~~~ ERROR - Could not create $$LP3D_QTQRC_FILE ~~~")
    }

    qt5_conf_d.files += \
        $$_PRO_FILE_PWD_/lpub3d-qtlibs.conf
    qt5_conf_d.path = $$LP3D_SO_CONF_DIR

    qt5_plugin_bearer.files += \
        $$[QT_INSTALL_PLUGINS]/bearer/libqgenericbearer.so
    qt5_plugin_bearer.path = $${LP3D_LIBDIR_QT5}/bearer

    qt5_plugin_iconengines.files += \
        $$[QT_INSTALL_PLUGINS]/iconengines/libqsvgicon.so
    qt5_plugin_iconengines.path = $${LP3D_LIBDIR_QT5}/iconengines

    qt5_plugin_imageformats.files += \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqgif.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqicns.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqico.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqjpeg.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqsvg.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqtga.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqtiff.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqwbmp.so \
        $$[QT_INSTALL_PLUGINS]/imageformats/libqwebp.so
    qt5_plugin_imageformats.path = $${LP3D_LIBDIR_QT5}/imageformats

    qt5_plugin_platforms.files += \
        $$[QT_INSTALL_PLUGINS]/platforms/libqlinuxfb.so \
        $$[QT_INSTALL_PLUGINS]/platforms/libqxcb.so
    qt5_plugin_platforms.path = $${LP3D_LIBDIR_QT5}/platforms

    qt5_qtlibs.files += \
        $$[QT_INSTALL_LIBS]/libQt5Core.so \
        $$[QT_INSTALL_LIBS]/libQt5Gui.so \
        $$[QT_INSTALL_LIBS]/libQt5Network.so \
        $$[QT_INSTALL_LIBS]/libQt5OpenGL.so \
        $$[QT_INSTALL_LIBS]/libQt5PrintSupport.so \
        $$[QT_INSTALL_LIBS]/libQt5Widgets.so
    qt5_qtlibs.path = $${LP3D_LIBDIR_QT5}/lib

    INSTALLS += \
        qt5_plugin_bearer \
        qt5_plugin_iconengines \
        qt5_plugin_imageformats \
        qt5_plugin_platforms \
        qt5_qtlibs \
        qt5_conf_d
  }

  equals(install_local_el_libs, 1): exists($$LP3D_LOCAL_LIBDIR_USR) {
    message("~~~ INSTALL LOCAL LIBS (OSMESA,LLVM,OPENEXR,LIBDRM) SPECIFIED ~~~")
    message("~~~ LOCAL LIBS SOURCE DIR: $$LP3D_LOCAL_LIBDIR_USR ~~~")

    LP3D_LDCONF_FILE   = $$_PRO_FILE_PWD_/lpub3d-libs.conf
    LP3D_LDCONF_LINES += $$LP3D_LIBDIR
    LP3D_LDCONF_LINES += $$LP3D_LIBDIR_LLVM

    !write_file($$LP3D_LDCONF_FILE, LP3D_LDCONF_LINES) {
      message("~~~ ERROR - Could not create $$LP3D_LDCONF_FILE ~~~")
    } else {
      message("~~~ FILE $$LP3D_LDCONF_FILE CREATED ~~~")
    }

    LP3D_PCS = $$system("find $${LP3D_LOCAL_LIBDIR_USR}/lib64/pkgconfig -type f")
    isEmpty(LP3D_PCS): message("~~~ ERROR - No .pc files found at $${LP3D_LOCAL_LIBDIR_USR}/lib64/pkgconfig ~~~")
    for(LP3D_PC, LP3D_PCS) {
        system("sed -i \"s,$${LP3D_LOCAL_LIBDIR_USR},$${LP3D_LIBDIR},g\" $${LP3D_PC} >/dev/null")
        message("~~~ UPDATE PC FILE $${LP3D_PC} PATH PREFIX TO $${LP3D_LIBDIR} ~~~")
    }

    local_el_llvm_conf_d.files += \
        $$LP3D_LOCAL_LIBDIR_ETC/ld.so.conf.d/llvm-x86_64.conf
    local_el_llvm_conf_d.path = $$LP3D_SO_CONF_DIR

    local_el_libdrm_rules.files += \
        $$LP3D_LOCAL_LIBDIR_USR/lib/udev/rules.d/91-drm-modeset.rules
    local_el_libdrm_rules.path = $$LP3D_DRM_RULES

    local_el_libs.files += \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libdrm.so.2.4.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libdrm_amdgpu.so.1.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libdrm_intel.so.1.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libdrm_nouveau.so.2.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libdrm_radeon.so.1.0.1 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libglapi.so.0.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libGLU.so.1.3.1 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libHalf.so.6.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libIex.so.6.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libIexMath.so.6.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libIlmImf.so.7.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libIlmThread.so.6.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libImath.so.6.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libkms.so.1.0.0 \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/libOSMesa.so.8.0.0
    local_el_libs.path = $$LP3D_LIBDIR

    local_el_libs_llvm.files += \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/llvm/BugpointPasses.so \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/llvm/libLLVM-3.4.so \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/llvm/libLTO.so \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/llvm/LLVMgold.so
    local_el_libs_llvm.path = $$LP3D_LIBDIR_LLVM

    local_el_libs_pkgconfig.files += \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/IlmBase.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/libdrm.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/libdrm_amdgpu.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/libdrm_intel.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/libdrm_nouveau.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/libdrm_radeon.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/libkms.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/OpenEXR.pc \
        $$LP3D_LOCAL_LIBDIR_USR/lib$${LIB_ARCH}/pkgconfig/osmesa.pc
    local_el_libs_pkgconfig.path = $$LP3D_LIBDIR_PKGCONFIG

    INSTALLS += \
        local_el_llvm_conf_d \
        local_el_libdrm_rules \
        local_el_libs \
        local_el_libs_llvm \
        local_el_libs_pkgconfig
  } else {
    equals(install_local_el_libs, 1): !exists($$LP3D_LOCAL_LIBDIR_USR): message("~~~ ERROR - $$LP3D_LOCAL_LIBDIR_USR not found ~~~")
    equals(install_local_el_libs, 1): !exists($$LP3D_LOCAL_LIBDIR_ETC): message("~~~ ERROR - $$LP3D_LOCAL_LIBDIR_ETC not found ~~~")
  }
}
