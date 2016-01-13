TARGET = GPXSee
QT += core \
    gui \
    network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
HEADERS += src/config.h \
    src/icons.h \
    src/gui.h \
    src/gpx.h \
    src/graph.h \
    src/track.h \
    src/parser.h \
    src/poi.h \
    src/rtree.h \
    src/ll.h \
    src/axisitem.h \
    src/poiitem.h \
    src/colorshop.h \
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
    src/nicenum.h
SOURCES += src/main.cpp \
    src/gui.cpp \
    src/gpx.cpp \
    src/graph.cpp \
    src/track.cpp \
    src/parser.cpp \
    src/poi.cpp \
    src/ll.cpp \
    src/axisitem.cpp \
    src/poiitem.cpp \
    src/colorshop.cpp \
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
    src/nicenum.cpp
RESOURCES += gpxsee.qrc
TRANSLATIONS = lang/gpxsee_cs.ts
macx:ICON = icons/gpxsee.icns
win32:RC_FILE = gpxsee.rc
