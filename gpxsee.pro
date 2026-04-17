unix:!macx:!android {
    TARGET = gpxsee
} else {
    TARGET = GPXSee
}
VERSION = 16.4

QT += core \
    gui \
    network \
    sql \
    concurrent \
    widgets \
    printsupport \
    positioning \
    svg \
    serialport \
    multimedia \
    multimediawidgets
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
}
# QZipReader
versionAtLeast(QT_VERSION, 6.6) {
    QT += core-private
} else {
    QT += gui-private
}

INCLUDEPATH += ./src
HEADERS += $$files(src/*.h, true)
SOURCES += $$files(src/*.cpp, true)
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
RESOURCES += gpxsee.qrc
TRANSLATIONS = $$files(lang/*.ts)

macx {
    LIBS += -lz
    RESOURCES += theme-grayscale.qrc

    ICON = icons/app/gpxsee.icns
    QMAKE_INFO_PLIST = pkg/mac/Info.plist
    locale.path = Contents/Resources/translations
    locale.files = $$files(lang/*.qm)
    crs.path = Contents/Resources
    crs.files = data/CRS
    maps.path = Contents/Resources
    maps.files = data/maps
    style.path = Contents/Resources
    style.files = data/style
    symbols.path = Contents/Resources/symbols
    symbols.files = $$files(icons/symbols/*.png)
    icons.path = Contents/Resources/icons
    icons.files = $$files(icons/formats/*.icns)
    QMAKE_BUNDLE_DATA += locale maps style symbols icons crs
}

win32 {
    RESOURCES += theme-color.qrc

    QMAKE_CXXFLAGS += /MP
    QMAKE_TARGET_DESCRIPTION = GPXSee
    QMAKE_TARGET_COPYRIGHT = Copyright (c) 2015-2026 Martin Tuma
    # If the icons order is not preserved, the file association breaks!
    RC_ICONS = icons/app/gpxsee.ico \
        icons/formats/gpx.ico \
        icons/formats/tcx.ico \
        icons/formats/kml.ico \
        icons/formats/fit.ico \
        icons/formats/igc.ico \
        icons/formats/nmea.ico \
        icons/formats/plt.ico \
        icons/formats/rte.ico \
        icons/formats/wpt.ico \
        icons/formats/loc.ico \
        icons/formats/slf.ico \
        icons/formats/json.ico \
        icons/formats/cup.ico \
        icons/formats/gpi.ico \
        icons/formats/sml.ico \
        icons/formats/img.ico \
        icons/formats/jnx.ico \
        icons/formats/kap.ico \
        icons/formats/map.ico \
        icons/formats/mbts.ico \
        icons/formats/rmap.ico \
        icons/formats/tba.ico \
        icons/formats/aqm.ico \
        icons/formats/sqlt.ico \
        icons/formats/ov2.ico \
        icons/formats/itn.ico \
        icons/formats/wld.ico \
        icons/formats/omd.ico \
        icons/formats/ghp.ico \
        icons/formats/qct.ico \
        icons/formats/trk.ico \
        icons/formats/gemf.ico \
        icons/formats/000.ico \
        icons/formats/031.ico \
        icons/formats/vtk.ico \
        icons/formats/vkx.ico \
        icons/formats/csa.ico \
        icons/formats/pmts.ico \
        icons/formats/pma.ico
    DEFINES += _USE_MATH_DEFINES \
        NOGDI
}

unix:!macx:!android {
    LIBS += -lz
    RESOURCES += theme-grayscale.qrc

    isEmpty(PREFIX):PREFIX = /usr/local
    maps.files = data/maps
    maps.path = $$PREFIX/share/gpxsee
    style.files = data/style
    style.path = $$PREFIX/share/gpxsee
    crs.files = data/CRS
    crs.path = $$PREFIX/share/gpxsee
    symbols.files = $$files(icons/symbols/*.png)
    symbols.path = $$PREFIX/share/gpxsee/symbols
    locale.files = $$files(lang/*.qm)
    locale.path = $$PREFIX/share/gpxsee/translations
    icon.files = icons/app/hicolor
    icon.path = $$PREFIX/share/icons
    desktop.files = pkg/linux/gpxsee.desktop
    desktop.path = $$PREFIX/share/applications
    mime.files = pkg/linux/gpxsee.xml
    mime.path = $$PREFIX/share/mime/packages
    appdata.files = pkg/linux/gpxsee.appdata.xml
    appdata.path = $$PREFIX/share/metainfo
    target.path = $$PREFIX/bin
    INSTALLS += target maps style crs symbols locale icon desktop mime appdata
}

android {
    QT += core5compat
    LIBS += -lz
    RESOURCES += theme-color.qrc

    defineReplace(versionCode) {
        segments = $$split(1, ".")
        for (segment, segments): \
            vCode = "$$first(vCode)$$format_number($$segment, width=3 zeropad)"
        contains(ANDROID_TARGET_ARCH, armeabi-v7a): \
            suffix = 0
        contains(ANDROID_TARGET_ARCH, arm64-v8a): \
            suffix = 1
        contains(ANDROID_TARGET_ARCH, x86): \
            suffix = 2
        contains(ANDROID_TARGET_ARCH, x86_64): \
            suffix = 3

        return($$first(vCode)$$first(suffix))
    }

    !include($$OPENSSL_PATH/openssl.pri) {
        message("OpenSSL not found, building without HTTPS support!")
    }

    ANDROID_TARGET_SDK_VERSION = 36
    ANDROID_VERSION_NAME = $$VERSION
    ANDROID_VERSION_CODE = $$versionCode($$ANDROID_VERSION_NAME)
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/pkg/android
    DISTFILES += \
        pkg/android/AndroidManifest.xml \
        pkg/android/build.gradle \
        pkg/android/res/values/libs.xml

    maps.files = data/maps
    maps.path = /assets
    style.files = data/style
    style.path = /assets
    crs.files = data/CRS
    crs.path = /assets
    symbols.files = $$files(icons/symbols/*.png)
    symbols.path = /assets/symbols
    translations.files = $$files(lang/*.qm)
    translations.path = /assets/translations
    INSTALLS += maps style crs symbols translations
}
