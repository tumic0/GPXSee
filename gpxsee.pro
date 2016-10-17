TARGET = GPXSee
VERSION = 2.20
QT += core \
    gui \
    network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
HEADERS += src/config.h \
    src/icons.h \
    src/gui.h \
    src/gpx.h \
    src/parser.h \
    src/poi.h \
    src/rtree.h \
    src/ll.h \
    src/axisitem.h \
    src/keys.h \
    src/slideritem.h \
    src/markeritem.h \
    src/infoitem.h \
    src/elevationgraph.h \
    src/speedgraph.h \
    src/sliderinfoitem.h \
    src/filebrowser.h \
    src/map.h \
    src/maplist.h \
    src/downloader.h \
    src/units.h \
    src/scaleitem.h \
    src/waypoint.h \
    src/track.h \
    src/graphview.h \
    src/trackpoint.h \
    src/waypointitem.h \
    src/palette.h \
    src/heartrategraph.h \
    src/range.h \
    src/cpuarch.h \
    src/settings.h \
    src/app.h \
    src/trackinfo.h \
    src/exportdialog.h \
    src/fileselectwidget.h \
    src/margins.h \
    src/temperaturegraph.h \
    src/graphtab.h \
    src/misc.h \
    src/trackitem.h \
    src/tooltip.h \
    src/route.h \
    src/routeitem.h \
    src/graphitem.h \
    src/graph.h \
    src/pathitem.h \
    src/pathview.h \
    src/griditem.h
SOURCES += src/main.cpp \
    src/gui.cpp \
    src/gpx.cpp \
    src/parser.cpp \
    src/poi.cpp \
    src/ll.cpp \
    src/axisitem.cpp \
    src/slideritem.cpp \
    src/markeritem.cpp \
    src/infoitem.cpp \
    src/elevationgraph.cpp \
    src/speedgraph.cpp \
    src/sliderinfoitem.cpp \
    src/filebrowser.cpp \
    src/map.cpp \
    src/maplist.cpp \
    src/downloader.cpp \
    src/scaleitem.cpp \
    src/track.cpp \
    src/graphview.cpp \
    src/waypointitem.cpp \
    src/palette.cpp \
    src/heartrategraph.cpp \
    src/range.cpp \
    src/app.cpp \
    src/trackinfo.cpp \
    src/exportdialog.cpp \
    src/fileselectwidget.cpp \
    src/temperaturegraph.cpp \
    src/trackpoint.cpp \
    src/misc.cpp \
    src/waypoint.cpp \
    src/trackitem.cpp \
    src/tooltip.cpp \
    src/route.cpp \
    src/routeitem.cpp \
    src/graphitem.cpp \
    src/pathitem.cpp \
    src/pathview.cpp \
    src/griditem.cpp
RESOURCES += gpxsee.qrc
TRANSLATIONS = lang/gpxsee_cs.ts
macx {
    ICON = icons/gpxsee.icns
    QMAKE_INFO_PLIST = Info.plist
    APP_RESOURCES.files = icons/gpx.icns \
        pkg/maps.txt
    APP_RESOURCES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += APP_RESOURCES
}
win32 {
    RC_ICONS = icons/gpxsee.ico \
        icons/gpx.ico
}
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
