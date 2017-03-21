TARGET = GPXSee
VERSION = 4.0
QT += core \
    gui \
    network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
lessThan(QT_VERSION, 5.4): QT += opengl
macx: QT += opengl
HEADERS += src/config.h \
    src/icons.h \
    src/gui.h \
    src/poi.h \
    src/rtree.h \
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
    src/onlinemap.h \
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
    src/griditem.h \
    src/data.h \
    src/gpxparser.h \
    src/tcxparser.h \
    src/parser.h \
    src/csvparser.h \
    src/coordinates.h \
    src/tile.h \
    src/rd.h \
    src/wgs84.h \
    src/kmlparser.h \
    src/trackdata.h \
    src/routedata.h \
    src/fitparser.h \
    src/format.h \
    src/path.h \
    src/assert.h \
    src/cadencegraph.h \
    src/powergraph.h \
    src/igcparser.h \
    src/nmeaparser.h \
    src/optionsdialog.h \
    src/colorbox.h \
    src/stylecombobox.h \
    src/opengl.h \
    src/timetype.h \
    src/emptymap.h \
    src/offlinemap.h \
    src/mapdir.h \
    src/matrix.h \
    src/tar.h
SOURCES += src/main.cpp \
    src/gui.cpp \
    src/poi.cpp \
    src/axisitem.cpp \
    src/slideritem.cpp \
    src/markeritem.cpp \
    src/infoitem.cpp \
    src/elevationgraph.cpp \
    src/speedgraph.cpp \
    src/sliderinfoitem.cpp \
    src/filebrowser.cpp \
    src/onlinemap.cpp \
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
    src/griditem.cpp \
    src/data.cpp \
    src/gpxparser.cpp \
    src/tcxparser.cpp \
    src/csvparser.cpp \
    src/coordinates.cpp \
    src/kmlparser.cpp \
    src/fitparser.cpp \
    src/format.cpp \
    src/graph.cpp \
    src/cadencegraph.cpp \
    src/powergraph.cpp \
    src/igcparser.cpp \
    src/path.cpp \
    src/nmeaparser.cpp \
    src/optionsdialog.cpp \
    src/colorbox.cpp \
    src/stylecombobox.cpp \
    src/emptymap.cpp \
    src/offlinemap.cpp \
    src/mapdir.cpp \
    src/matrix.cpp \
    src/tar.cpp
RESOURCES += gpxsee.qrc
TRANSLATIONS = lang/gpxsee_cs.ts \
    lang/gpxsee_sv.ts
macx {
    ICON = icons/gpxsee.icns
    QMAKE_INFO_PLIST = Info.plist
    APP_RESOURCES.files = icons/gpx.icns \
        icons/tcx.icns \
        icons/kml.icns \
        icons/fit.icns \
        icons/igc.icns \
        icons/nmea.icns \
        pkg/maps.txt
    APP_RESOURCES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += APP_RESOURCES
}
win32 {
    RC_ICONS = icons/gpxsee.ico \
        icons/gpx.ico \
        icons/tcx.ico \
        icons/kml.ico \
        icons/fit.ico \
        icons/igc.ico \
        icons/nmea.ico
}
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
