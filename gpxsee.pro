unix:!macx {
    TARGET = gpxsee
} else {
    TARGET = GPXSee
}
VERSION = 7.30

QT += core \
    gui \
    network \
    sql \
    concurrent
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    QT += printsupport
}
lessThan(QT_VERSION, 5.4.0) {QT += opengl}

INCLUDEPATH += ./src
HEADERS += src/common/config.h \
    src/GUI/graphicsscene.h \
    src/GUI/mapaction.h \
    src/GUI/popup.h \
    src/common/garmin.h \
    src/common/staticassert.h \
    src/common/coordinates.h \
    src/common/range.h \
    src/common/rectc.h \
    src/common/wgs84.h \
    src/common/util.h \
    src/common/rtree.h \
    src/common/kv.h \
    src/common/greatcircle.h \
    src/common/programpaths.h \
    src/common/tifffile.h \
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
    src/GUI/gearratiograph.h \
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
    src/GUI/gearratiographitem.h \
    src/GUI/oddspinbox.h \
    src/GUI/settings.h \
    src/GUI/cpuarch.h \
    src/GUI/searchpointer.h \
    src/GUI/mapview.h \
    src/GUI/font.h \
    src/GUI/areaitem.h \
    src/data/link.h \
    src/map/IMG/bitmapline.h \
    src/map/IMG/bitstream.h \
    src/map/IMG/deltastream.h \
    src/map/IMG/gmap.h \
    src/map/IMG/huffmanstream.h \
    src/map/IMG/huffmantable.h \
    src/map/IMG/mapdata.h \
    src/map/IMG/rastertile.h \
    src/map/IMG/textpathitem.h \
    src/map/IMG/textpointitem.h \
    src/map/projection.h \
    src/map/ellipsoid.h \
    src/map/datum.h \
    src/map/webmercator.h \
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
    src/map/ozimap.h \
    src/map/tar.h \
    src/map/ozf.h \
    src/map/atlas.h \
    src/map/matrix.h \
    src/map/geotiff.h \
    src/map/pcs.h \
    src/map/transform.h \
    src/map/mapfile.h \
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
    src/map/pointd.h \
    src/map/rectd.h \
    src/map/geocentric.h \
    src/map/mercator.h \
    src/map/jnxmap.h \
    src/map/krovak.h \
    src/map/geotiffmap.h \
    src/map/image.h \
    src/map/mbtilesmap.h \
    src/map/osm.h \
    src/map/polarstereographic.h \
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
    src/data/gpxparser.h \
    src/data/tcxparser.h \
    src/data/csvparser.h \
    src/data/kmlparser.h \
    src/data/fitparser.h \
    src/data/igcparser.h \
    src/data/nmeaparser.h \
    src/data/oziparsers.h \
    src/data/locparser.h \
    src/data/slfparser.h \
    src/data/dem.h \
    src/data/polygon.h \
    src/data/area.h \
    src/map/obliquestereographic.h \
    src/GUI/coordinatesitem.h \
    src/map/rmap.h \
    src/map/calibrationpoint.h \
    src/map/color.h \
    src/data/exifparser.h \
    src/data/imageinfo.h \
    src/map/imgmap.h \
    src/map/IMG/img.h \
    src/map/IMG/subfile.h \
    src/map/IMG/trefile.h \
    src/map/IMG/rgnfile.h \
    src/map/IMG/lblfile.h \
    src/map/IMG/vectortile.h \
    src/map/IMG/subdiv.h \
    src/map/IMG/style.h \
    src/map/IMG/netfile.h \
    src/GUI/limitedcombobox.h \
    src/GUI/pathtickitem.h \
    src/map/IMG/textitem.h \
    src/map/IMG/label.h \
    src/data/csv.h \
    src/data/cupparser.h \
    src/data/gpiparser.h \
    src/data/address.h \
    src/data/smlparser.h
SOURCES += src/main.cpp \
    src/GUI/popup.cpp \
    src/common/coordinates.cpp \
    src/common/rectc.cpp \
    src/common/range.cpp \
    src/common/util.cpp \
    src/common/greatcircle.cpp \
    src/common/programpaths.cpp \
    src/common/tifffile.cpp \
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
    src/GUI/gearratiograph.cpp \
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
    src/GUI/gearratiographitem.cpp \
    src/GUI/mapview.cpp \
    src/GUI/areaitem.cpp \
    src/data/waypoint.cpp \
    src/map/IMG/bitmapline.cpp \
    src/map/IMG/bitstream.cpp \
    src/map/IMG/deltastream.cpp \
    src/map/IMG/gmap.cpp \
    src/map/IMG/huffmanstream.cpp \
    src/map/IMG/huffmantable.cpp \
    src/map/IMG/mapdata.cpp \
    src/map/IMG/rastertile.cpp \
    src/map/IMG/textpathitem.cpp \
    src/map/IMG/textpointitem.cpp \
    src/map/maplist.cpp \
    src/map/onlinemap.cpp \
    src/map/downloader.cpp \
    src/map/emptymap.cpp \
    src/map/ozimap.cpp \
    src/map/tar.cpp \
    src/map/atlas.cpp \
    src/map/ozf.cpp \
    src/map/matrix.cpp \
    src/map/ellipsoid.cpp \
    src/map/datum.cpp \
    src/map/webmercator.cpp \
    src/map/transversemercator.cpp \
    src/map/utm.cpp \
    src/map/lambertconic.cpp \
    src/map/albersequal.cpp \
    src/map/lambertazimuthal.cpp \
    src/map/geotiff.cpp \
    src/map/pcs.cpp \
    src/map/transform.cpp \
    src/map/mapfile.cpp \
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
    src/map/geocentric.cpp \
    src/map/mercator.cpp \
    src/map/jnxmap.cpp \
    src/map/krovak.cpp \
    src/map/map.cpp \
    src/map/geotiffmap.cpp \
    src/map/image.cpp \
    src/map/mbtilesmap.cpp \
    src/map/osm.cpp \
    src/map/polarstereographic.cpp \
    src/map/rectd.cpp \
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
    src/data/oziparsers.cpp \
    src/data/locparser.cpp \
    src/data/slfparser.cpp \
    src/data/dem.cpp \
    src/data/polygon.cpp \
    src/map/obliquestereographic.cpp \
    src/GUI/coordinatesitem.cpp \
    src/map/rmap.cpp \
    src/data/exifparser.cpp \
    src/map/imgmap.cpp \
    src/map/IMG/img.cpp \
    src/map/IMG/subfile.cpp \
    src/map/IMG/trefile.cpp \
    src/map/IMG/rgnfile.cpp \
    src/map/IMG/lblfile.cpp \
    src/map/IMG/vectortile.cpp \
    src/map/IMG/style.cpp \
    src/map/IMG/netfile.cpp \
    src/GUI/pathtickitem.cpp \
    src/map/IMG/textitem.cpp \
    src/data/csv.cpp \
    src/data/cupparser.cpp \
    src/GUI/graphicsscene.cpp \
    src/data/gpiparser.cpp \
    src/data/smlparser.cpp

greaterThan(QT_MAJOR_VERSION, 4) {
    HEADERS += src/data/geojsonparser.h
    SOURCES += src/data/geojsonparser.cpp
}
greaterThan(QT_VERSION, 5.1.0) {
    HEADERS += src/GUI/timezoneinfo.h
}

DEFINES += APP_VERSION=\\\"$$VERSION\\\" \
    QT_NO_DEPRECATED_WARNINGS
DEFINES *= QT_USE_QSTRINGBUILDER

RESOURCES += gpxsee.qrc
TRANSLATIONS = lang/gpxsee_en.ts \
    lang/gpxsee_cs.ts \
    lang/gpxsee_sv.ts \
    lang/gpxsee_de.ts \
    lang/gpxsee_ru.ts \
    lang/gpxsee_fi.ts \
    lang/gpxsee_fr.ts \
    lang/gpxsee_pl.ts \
    lang/gpxsee_nb.ts \
    lang/gpxsee_da.ts \
    lang/gpxsee_tr.ts \
    lang/gpxsee_es.ts \
    lang/gpxsee_pt_BR.ts \
    lang/gpxsee_uk.ts \
    lang/gpxsee_hu.ts

macx {
    ICON = icons/gpxsee.icns
    QMAKE_INFO_PLIST = pkg/Info.plist
    locale.path = Contents/Resources/translations
    locale.files = lang/gpxsee_en.qm \
        lang/gpxsee_cs.qm \
        lang/gpxsee_de.qm \
        lang/gpxsee_fi.qm \
        lang/gpxsee_fr.qm \
        lang/gpxsee_ru.qm \
        lang/gpxsee_sv.qm \
        lang/gpxsee_pl.qm \
        lang/gpxsee_nb.qm \
        lang/gpxsee_da.qm \
        lang/gpxsee_tr.qm \
        lang/gpxsee_es.qm \
        lang/gpxsee_pt_BR.qm \
        lang/gpxsee_uk.qm \
        lang/gpxsee_hu.qm
    csv.path = Contents/Resources
    csv.files = pkg/csv
    maps.path = Contents/Resources
    maps.files = pkg/maps
    icons.path = Contents/Resources/icons
    icons.files = icons/formats/gpx.icns \
        icons/formats/tcx.icns \
        icons/formats/kml.icns \
        icons/formats/fit.icns \
        icons/formats/igc.icns \
        icons/formats/nmea.icns \
        icons/formats/plt.icns \
        icons/formats/rte.icns \
        icons/formats/wpt.icns \
        icons/formats/loc.icns \
        icons/formats/slf.icns \
        icons/formats/json.icns \
        icons/formats/cup.icns \
        icons/formats/gpi.icns \
        icons/formats/sml.icns
    QMAKE_BUNDLE_DATA += locale maps icons csv
}

win32 {
    RC_ICONS = icons/gpxsee.ico \
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
        icons/formats/sml.ico
    DEFINES += _USE_MATH_DEFINES \
        NOGDI
}

unix:!macx {
    isEmpty(PREFIX):PREFIX = /usr/local
    DEFINES += PREFIX=\\\"$$PREFIX\\\"

    maps.files = pkg/maps/*
    maps.path = $$PREFIX/share/gpxsee/maps
    csv.files = pkg/csv/*
    csv.path = $$PREFIX/share/gpxsee/csv
    locale.files = lang/*.qm
    locale.path = $$PREFIX/share/gpxsee/translations
    icon.files = icons/gpxsee.png
    icon.path = $$PREFIX/share/pixmaps
    desktop.files = pkg/gpxsee.desktop
    desktop.path = $$PREFIX/share/applications
    mime.files = pkg/gpxsee.xml
    mime.path = $$PREFIX/share/mime/packages
    target.path = $$PREFIX/bin
    INSTALLS += target maps csv locale icon desktop mime
}
