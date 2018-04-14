TARGET = GPXSee
VERSION = 5.6
QT += core \
    gui \
    network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
lessThan(QT_VERSION, 5.4): QT += opengl
macx: QT += opengl
INCLUDEPATH += ./src
HEADERS += src/config.h \
    src/common/staticassert.h \
    src/common/coordinates.h \
    src/common/range.h \
    src/common/rectc.h \
    src/common/wgs84.h \
    src/common/str2int.h \
    src/GUI/app.h \
    src/GUI/icons.h \
    src/GUI/gui.h \
    src/GUI/axisitem.h \
    src/GUI/keys.h \
    src/GUI/slideritem.h \
    src/GUI/markeritem.h \
    src/GUI/infoitem.h \
    src/GUI/elevationgraph.h \
    src/GUI/speedgraph.h \
    src/GUI/sliderinfoitem.h \
    src/GUI/filebrowser.h \
    src/GUI/units.h \
    src/GUI/scaleitem.h \
    src/GUI/graphview.h \
    src/GUI/waypointitem.h \
    src/GUI/palette.h \
    src/GUI/heartrategraph.h \
    src/GUI/trackinfo.h \
    src/GUI/exportdialog.h \
    src/GUI/fileselectwidget.h \
    src/GUI/margins.h \
    src/GUI/temperaturegraph.h \
    src/GUI/graphtab.h \
    src/GUI/trackitem.h \
    src/GUI/tooltip.h \
    src/GUI/routeitem.h \
    src/GUI/graphitem.h \
    src/GUI/pathitem.h \
    src/GUI/griditem.h \
    src/GUI/format.h \
    src/GUI/cadencegraph.h \
    src/GUI/powergraph.h \
    src/GUI/optionsdialog.h \
    src/GUI/colorbox.h \
    src/GUI/stylecombobox.h \
    src/GUI/opengl.h \
    src/GUI/timetype.h \
    src/GUI/percentslider.h \
    src/GUI/elevationgraphitem.h \
    src/GUI/speedgraphitem.h \
    src/GUI/heartrategraphitem.h \
    src/GUI/temperaturegraphitem.h \
    src/GUI/cadencegraphitem.h \
    src/GUI/powergraphitem.h \
    src/GUI/oddspinbox.h \
    src/GUI/settings.h \
    src/GUI/nicenum.h \
    src/GUI/cpuarch.h \
    src/GUI/searchpointer.h \
    src/GUI/mapview.h \
    src/map/projection.h \
    src/map/ellipsoid.h \
    src/map/datum.h \
    src/map/mercator.h \
    src/map/transversemercator.h \
    src/map/latlon.h \
    src/map/utm.h \
    src/map/lambertconic.h \
    src/map/lambertazimuthal.h \
    src/map/albersequal.h \
    src/map/map.h \
    src/map/maplist.h \
    src/map/onlinemap.h \
    src/map/downloader.h \
    src/map/tile.h \
    src/map/emptymap.h \
    src/map/offlinemap.h \
    src/map/tar.h \
    src/map/ozf.h \
    src/map/atlas.h \
    src/map/matrix.h \
    src/map/geotiff.h \
    src/map/pcs.h \
    src/map/transform.h \
    src/map/mapfile.h \
    src/map/tifffile.h \
    src/data/graph.h \
    src/data/poi.h \
    src/data/waypoint.h \
    src/data/track.h \
    src/data/route.h \
    src/data/trackpoint.h \
    src/data/data.h \
    src/data/parser.h \
    src/data/trackdata.h \
    src/data/routedata.h \
    src/data/path.h \
    src/data/rtree.h \
    src/data/gpxparser.h \
    src/data/tcxparser.h \
    src/data/csvparser.h \
    src/data/kmlparser.h \
    src/data/fitparser.h \
    src/data/igcparser.h \
    src/data/nmeaparser.h \
    src/map/gcs.h \
    src/map/angularunits.h \
    src/map/primemeridian.h \
    src/map/linearunits.h \
    src/map/ct.h \
    src/map/mapsource.h \
    src/map/tileloader.h \
    src/map/wmtsmap.h \
    src/map/wmts.h \
    src/map/wmsmap.h \
    src/map/wms.h \
    src/map/crs.h \
    src/map/coordinatesystem.h \
    src/data/pltparser.h \
    src/data/date.h \
    src/data/wptparser.h \
    src/data/rteparser.h
SOURCES += src/main.cpp \
    src/common/coordinates.cpp \
    src/common/rectc.cpp \
    src/common/range.cpp \
    src/common/str2int.cpp \
    src/GUI/app.cpp \
    src/GUI/gui.cpp \
    src/GUI/axisitem.cpp \
    src/GUI/slideritem.cpp \
    src/GUI/markeritem.cpp \
    src/GUI/infoitem.cpp \
    src/GUI/elevationgraph.cpp \
    src/GUI/speedgraph.cpp \
    src/GUI/sliderinfoitem.cpp \
    src/GUI/filebrowser.cpp \
    src/GUI/scaleitem.cpp \
    src/GUI/graphview.cpp \
    src/GUI/waypointitem.cpp \
    src/GUI/palette.cpp \
    src/GUI/heartrategraph.cpp \
    src/GUI/trackinfo.cpp \
    src/GUI/exportdialog.cpp \
    src/GUI/fileselectwidget.cpp \
    src/GUI/temperaturegraph.cpp \
    src/GUI/trackitem.cpp \
    src/GUI/tooltip.cpp \
    src/GUI/routeitem.cpp \
    src/GUI/graphitem.cpp \
    src/GUI/pathitem.cpp \
    src/GUI/griditem.cpp \
    src/GUI/format.cpp \
    src/GUI/cadencegraph.cpp \
    src/GUI/powergraph.cpp \
    src/GUI/optionsdialog.cpp \
    src/GUI/colorbox.cpp \
    src/GUI/stylecombobox.cpp \
    src/GUI/oddspinbox.cpp \
    src/GUI/percentslider.cpp \
    src/GUI/elevationgraphitem.cpp \
    src/GUI/speedgraphitem.cpp \
    src/GUI/heartrategraphitem.cpp \
    src/GUI/temperaturegraphitem.cpp \
    src/GUI/cadencegraphitem.cpp \
    src/GUI/powergraphitem.cpp \
    src/GUI/nicenum.cpp \
    src/GUI/mapview.cpp \
    src/map/maplist.cpp \
    src/map/onlinemap.cpp \
    src/map/downloader.cpp \
    src/map/emptymap.cpp \
    src/map/offlinemap.cpp \
    src/map/tar.cpp \
    src/map/atlas.cpp \
    src/map/ozf.cpp \
    src/map/matrix.cpp \
    src/map/ellipsoid.cpp \
    src/map/datum.cpp \
    src/map/mercator.cpp \
    src/map/transversemercator.cpp \
    src/map/utm.cpp \
    src/map/lambertconic.cpp \
    src/map/albersequal.cpp \
    src/map/lambertazimuthal.cpp \
    src/map/geotiff.cpp \
    src/map/pcs.cpp \
    src/map/transform.cpp \
    src/map/mapfile.cpp \
    src/map/tifffile.cpp \
    src/data/data.cpp \
    src/data/poi.cpp \
    src/data/track.cpp \
    src/data/route.cpp \
    src/data/path.cpp \
    src/data/gpxparser.cpp \
    src/data/tcxparser.cpp \
    src/data/csvparser.cpp \
    src/data/kmlparser.cpp \
    src/data/fitparser.cpp \
    src/data/igcparser.cpp \
    src/data/nmeaparser.cpp \
    src/map/projection.cpp \
    src/map/gcs.cpp \
    src/map/angularunits.cpp \
    src/map/primemeridian.cpp \
    src/map/linearunits.cpp \
    src/map/mapsource.cpp \
    src/map/tileloader.cpp \
    src/map/wmtsmap.cpp \
    src/map/wmts.cpp \
    src/map/wmsmap.cpp \
    src/map/wms.cpp \
    src/map/crs.cpp \
    src/map/coordinatesystem.cpp \
    src/data/pltparser.cpp \
    src/data/wptparser.cpp \
    src/data/rteparser.cpp
RESOURCES += gpxsee.qrc
TRANSLATIONS = lang/gpxsee_cs.ts \
    lang/gpxsee_sv.ts \
    lang/gpxsee_de.ts \
    lang/gpxsee_ru.ts \
    lang/gpxsee_fi.ts \
    lang/gpxsee_fr.ts
macx {
    ICON = icons/gpxsee.icns
    QMAKE_INFO_PLIST = pkg/Info.plist
    LOCALE.path = Contents/Resources/translations
    LOCALE.files = lang/gpxsee_cs.qm \
        lang/gpxsee_de.qm \
        lang/gpxsee_fi.qm \
        lang/gpxsee_fr.qm \
        lang/gpxsee_ru.qm \
        lang/gpxsee_sv.qm
    CSV.path = Contents/Resources
    CSV.files = pkg/csv
    MAPS.path = Contents/Resources
    MAPS.files = pkg/maps
    ICONS.path = Contents/Resources/icons
    ICONS.files = icons/tcx.icns \
        icons/kml.icns \
        icons/fit.icns \
        icons/igc.icns \
        icons/nmea.icns \
        icons/plt.icns \
        icons/rte.icns \
        icons/wpt.icns
    QMAKE_BUNDLE_DATA += LOCALE MAPS ICONS CSV
}
win32 {
    RC_ICONS = icons/gpxsee.ico \
        icons/gpx.ico \
        icons/tcx.ico \
        icons/kml.ico \
        icons/fit.ico \
        icons/igc.ico \
        icons/nmea.ico \
        icons/plt.ico \
        icons/rte.ico \
        icons/wpt.ico
}
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
