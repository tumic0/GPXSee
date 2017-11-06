TARGET = GPXSee
VERSION = 4.16
QT += core \
    gui \
    network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
lessThan(QT_VERSION, 5.4): QT += opengl
macx: QT += opengl

# TODO: Modify sources to have proper includes
INCLUDEPATH += src/util
INCLUDEPATH += src/projection
INCLUDEPATH += src/map
INCLUDEPATH += src/geodata
INCLUDEPATH += src/importer
INCLUDEPATH += src/gui

# Util
HEADERS +=                                      \
    src/util/downloader.h                       \
    src/util/tar.h                              \
    src/util/ellipsoid.h                        \
    src/util/matrix.h                           \
    src/util/margins.h                          \
    src/util/misc.h                             \
    src/util/palette.h                          \
    src/util/range.h                            \
    src/util/filebrowser.h                      \
    src/util/cpuarch.h                          \
    src/util/rd.h                               \
    src/util/staticassert.h                     \
    src/util/units.h                            \
    src/util/tile.h                             \
    src/util/rtree.h                            \
    src/util/searchpointer.h                    \
    src/util/timetype.h                         \
    src/util/wgs84.h                            \
    src/util/config.h                           \
    src/util/settings.h                         \

SOURCES +=                                      \
    src/util/downloader.cpp                     \
    src/util/tar.cpp                            \
    src/util/ellipsoid.cpp                      \
    src/util/matrix.cpp                         \
    src/util/margins.cpp                        \
    src/util/misc.cpp                           \
    src/util/palette.cpp                        \
    src/util/range.cpp                          \
    src/util/filebrowser.cpp                    \

# Projection
HEADERS +=                                      \
    src/projection/projection.h                 \
    src/projection/albersequal.h                \
    src/projection/lambertconic.h               \
    src/projection/mercator.h                   \
    src/projection/transversemercator.h         \
    src/projection/utm.h                        \
    src/projection/latlon.h                     \

SOURCES +=                                      \
    src/projection/albersequal.cpp              \
    src/projection/lambertconic.cpp             \
    src/projection/mercator.cpp                 \
    src/projection/transversemercator.cpp       \
    src/projection/utm.cpp                      \

# Maps
HEADERS +=                                      \
    src/map/map.h                               \
    src/map/maplist.h                           \
    src/map/atlas.h                             \
    src/map/emptymap.h                          \
    src/map/onlinemap.h                         \
    src/map/offlinemap.h                        \
    src/map/ozf.h                               \
    src/map/rectc.h                             \
    src/map/datum.h                             \

SOURCES +=                                      \
    src/map/maplist.cpp                         \
    src/map/atlas.cpp                           \
    src/map/emptymap.cpp                        \
    src/map/onlinemap.cpp                       \
    src/map/offlinemap.cpp                      \
    src/map/ozf.cpp                             \
    src/map/rectc.cpp                           \
    src/map/datum.cpp                           \

# Geodata
HEADERS +=                                      \
    src/geodata/coordinates.h                   \
    src/geodata/format.h                        \
    src/geodata/geodata.h                       \
    src/geodata/path.h                          \
    src/geodata/pathitem.h                      \
    src/geodata/poi.h                           \
    src/geodata/route.h                         \
    src/geodata/routeitem.h                     \
    src/geodata/routeitemsmodel.h               \
    src/geodata/track.h                         \
    src/geodata/trackinfo.h                     \
    src/geodata/trackitem.h                     \
    src/geodata/trackitemsmodel.h               \
    src/geodata/trackpoint.h                    \
    src/geodata/waypoint.h                      \
    src/geodata/waypointitem.h                  \
    src/geodata/waypointitemsmodel.h            \
    src/geodata/data.h                          \

SOURCES +=                                      \
    src/geodata/coordinates.cpp                 \
    src/geodata/format.cpp                      \
    src/geodata/path.cpp                        \
    src/geodata/pathitem.cpp                    \
    src/geodata/poi.cpp                         \
    src/geodata/route.cpp                       \
    src/geodata/routeitem.cpp                   \
    src/geodata/routeitemsmodel.cpp             \
    src/geodata/track.cpp                       \
    src/geodata/trackinfo.cpp                   \
    src/geodata/trackitem.cpp                   \
    src/geodata/trackitemsmodel.cpp             \
    src/geodata/trackpoint.cpp                  \
    src/geodata/waypoint.cpp                    \
    src/geodata/waypointitem.cpp                \
    src/geodata/waypointitemsmodel.cpp          \
    src/geodata/data.cpp                        \

# Importers
HEADERS +=                                      \
    src/importer/parser.h                       \
    src/importer/csvparser.h                    \
    src/importer/fitparser.h                    \
    src/importer/gpxparser.h                    \
    src/importer/igcparser.h                    \
    src/importer/kmlparser.h                    \
    src/importer/nmeaparser.h                   \
    src/importer/tcxparser.h                    \

SOURCES +=                                      \
    src/importer/csvparser.cpp                  \
    src/importer/fitparser.cpp                  \
    src/importer/gpxparser.cpp                  \
    src/importer/igcparser.cpp                  \
    src/importer/kmlparser.cpp                  \
    src/importer/nmeaparser.cpp                 \
    src/importer/tcxparser.cpp                  \

# GUI
HEADERS +=                                      \
    src/gui/gui.h                               \
    src/gui/app.h                               \
    src/gui/exportdialog.h                      \
    src/gui/fileselectwidget.h                  \
    src/gui/percentslider.h                     \
    src/gui/tooltip.h                           \
    src/gui/axisitem.h                          \
    src/gui/pathview.h                          \
    src/gui/graph.h                             \
    src/gui/graphtab.h                          \
    src/gui/graphitem.h                         \
    src/gui/graphview.h                         \
    src/gui/powergraph.h                        \
    src/gui/powergraphitem.h                    \
    src/gui/elevationgraph.h                    \
    src/gui/elevationgraphitem.h                \
    src/gui/temperaturegraph.h                  \
    src/gui/temperaturegraphitem.h              \
    src/gui/speedgraph.h                        \
    src/gui/speedgraphitem.h                    \
    src/gui/cadencegraph.h                      \
    src/gui/cadencegraphitem.h                  \
    src/gui/heartrategraph.h                    \
    src/gui/heartrategraphitem.h                \
    src/gui/optionsdialog.h                     \
    src/gui/colorbox.h                          \
    src/gui/griditem.h                          \
    src/gui/infoitem.h                          \
    src/gui/keys.h                              \
    src/gui/markeritem.h                        \
    src/gui/oddspinbox.h                        \
    src/gui/opengl.h                            \
    src/gui/scaleitem.h                         \
    src/gui/sliderinfoitem.h                    \
    src/gui/slideritem.h                        \
    src/gui/stylecombobox.h                     \
    src/gui/datalistview.h                      \
    src/gui/icons.h                             \

SOURCES +=                                      \
    src/gui/gui.cpp                             \
    src/gui/app.cpp                             \
    src/gui/exportdialog.cpp                    \
    src/gui/fileselectwidget.cpp                \
    src/gui/percentslider.cpp                   \
    src/gui/tooltip.cpp                         \
    src/gui/axisitem.cpp                        \
    src/gui/pathview.cpp                        \
    src/gui/graph.cpp                           \
    src/gui/graphitem.cpp                       \
    src/gui/graphview.cpp                       \
    src/gui/powergraph.cpp                      \
    src/gui/powergraphitem.cpp                  \
    src/gui/elevationgraph.cpp                  \
    src/gui/elevationgraphitem.cpp              \
    src/gui/temperaturegraph.cpp                \
    src/gui/temperaturegraphitem.cpp            \
    src/gui/speedgraph.cpp                      \
    src/gui/speedgraphitem.cpp                  \
    src/gui/cadencegraph.cpp                    \
    src/gui/cadencegraphitem.cpp                \
    src/gui/heartrategraph.cpp                  \
    src/gui/heartrategraphitem.cpp              \
    src/gui/optionsdialog.cpp                   \
    src/gui/colorbox.cpp                        \
    src/gui/griditem.cpp                        \
    src/gui/infoitem.cpp                        \
    src/gui/markeritem.cpp                      \
    src/gui/oddspinbox.cpp                      \
    src/gui/scaleitem.cpp                       \
    src/gui/sliderinfoitem.cpp                  \
    src/gui/slideritem.cpp                      \
    src/gui/stylecombobox.cpp                   \
    src/gui/datalistview.cpp                    \

# Main
SOURCES += src/main.cpp \

RESOURCES += gpxsee.qrc
TRANSLATIONS = lang/gpxsee_cs.ts \
    lang/gpxsee_sv.ts \
    lang/gpxsee_de.ts
macx {
    ICON = icons/gpxsee.icns
    QMAKE_INFO_PLIST = Info.plist
    APP_RESOURCES.files = icons/gpx.icns \
        icons/tcx.icns \
        icons/kml.icns \
        icons/fit.icns \
        icons/igc.icns \
        icons/nmea.icns \
        pkg/maps.txt \
        pkg/ellipsoids.csv \
        pkg/datums.csv
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
