TARGET = GPXSee
QT += core \
	gui
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
    src/markeritem.h
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
    src/markeritem.cpp
RESOURCES += gpxsee.qrc
TRANSLATIONS = lang/gpxsee_cs.ts
macx:ICON = icons/gpxsee.icns
win32:RC_FILE = gpxsee.rc
