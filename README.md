# GPXSee
GPXSee is a Qt-based GPS log file viewer and analyzer that supports all common
GPS log file formats.

## Features
* Opens GPX, TCX, FIT, KML, NMEA, IGC, CUP, SIGMA SLF, Suunto SML, LOC, GeoJSON,
  OziExplorer (PLT, RTE, WPT), Garmin GPI&CSV, TomTom OV2&ITN, ONmove OMD/GHP,
  TwoNav (TRK, RTE, WPT) and geotagged JPEG files.
* User-definable online maps (OpenStreetMap/Google tiles, WMTS, WMS, TMS,
  QuadTiles).
* Offline maps (MBTiles, OziExplorer maps, TrekBuddy maps/atlases,
  Garmin IMG/GMAP & JNX maps, TwoNav RMaps, GeoTIFF images, BSB charts,
  ENC charts, KMZ maps, AlpineQuest maps, Locus/OsmAnd/RMaps SQLite maps,
  Mapsforge vector maps, QCT maps, GEMF maps, Osmdroid SQLite maps, Orux maps,
  ESRI World-File georeferenced images).
* Elevation, speed, heart rate, cadence, power, temperature and gear ratio/shifts
  graphs.
* Support for DEM files (SRTM HGT).
* Support for multiple tracks in one view.
* Support for POI files.
* Print/export to PDF/PNG.
* Full-screen mode.
* HiDPI/Retina displays & maps support.
* Real-time GPS position.
* Windows, macOS, Linux and Android builds.

![GPXSee - Linux](https://a.fsdn.com/con/app/proj/gpxsee/screenshots/linux2.png)

## Build
Build requirements:
* Qt5 >= 5.11 or Qt6 >= 6.2 (Android builds require Qt6)
* C++11 or newer compiler (tested: msvc2019, gcc 7.5.0, clang/Apple LLVM version
  10.0.0)

Build steps:
```shell
lrelease gpxsee.pro
qmake gpxsee.pro
make # nmake on windows
```



## Download
* [Windows & OS X builds](https://sourceforge.net/projects/gpxsee)
* [Linux packages](https://software.opensuse.org/download.html?project=home%3Atumic%3AGPXSee&package=gpxsee)
* [Android APKs](https://play.google.com/store/apps/details?id=org.gpxsee.gpxsee)

## Changelog
[Changelog](https://build.opensuse.org/package/view_file/home:tumic:GPXSee/gpxsee/gpxsee.changes)

## Homepage
[https://www.gpxsee.org](https://www.gpxsee.org)

## Maps
[GPXSee maps repository](https://github.com/tumic0/GPXSee-maps)

## Translations
GPXSee uses [Weblate](https://hosted.weblate.org/projects/gpxsee/translations/)
for translations.

## License
GPXSee is licensed under GPL-3.0 (only). However, some 3rd party parts are using
different, GPL compatible, licenses:

* [Oxygen icons](icons/GUI) - LGPLv3
* [Mapbox Maki icons](icons/map/POI) - CC0
* [Map Icons Collection](icons/symbols) - CC BY SA 3.0
* [RTree implementation](src/common/rtree.h) - Public domain
* [Albers](src/map/proj/albersequal.cpp), [Geocentric](src/map/geocentric.cpp),
  [LCC](src/map/proj/lambertconic.cpp), [Mercator](src/map/proj/mercator.cpp),
  [Polar Stereographic](src/map/proj/polarstereographic.cpp),
  [Polyconic](src/map/proj/polyconic.cpp) and
  [Transverse Mercator](src/map/proj/transversemercator.cpp) projections - NIMA
  Source Code Disclaimer
* [Projection parameters CSV files](pkg/csv) - BSD/EPSG/Public domain
* [Mapsforge render theme](data/default.xml) and its [icons](icons/map/mapsforge) - LGPLv3

## Ubuntu dev envionment

sudo apt install -y build-essential qttools5-dev-tools qtbase5-private-dev qtpositioning5-dev
sudo apt install qt6-base-dev qt6-base-private-dev qt6-serialbus-dev qt6-svg-dev qt6-positioning-dev


## Code Architecture

The UI lives in gui.cpp
It for example:
* handles screenChanged() events incl. dpi changes
* loadInitialMaps() and loadInitialPOIs()
* changes a map on menu click with mapAction() with Map.trigger() calling mapChanged calling setMap
can use mapAction("alps/esloAlpsW") to force one
to retrieve the map from the action
Map *map = mapaction->data().value<Map*>();

on mapLoadedDir,
_mapView->loadMaps(actions) is called initially to read the map files and populate the actions

The gui is a QWidget and includes a MapView defined in mapview.cpp (pointing back to its parent)

It inherits from QGraphicsView.

QGraphicsView visualizes the contents of a QGraphicsScene in a scrollable viewport.
I can use CPU raster rendering or GPU if backed by a QGLWidget.

analogy to the recording of a movie, the QGraphicsView would be the camera, the QGraphicsScene represents what is recorded, ie the scene. The scene is delimited by the sceneRect, if the camera is very close to the scene, its limits will not be observed, but if the camera is far away, the scenerect projection in the camera will not occupy the whole screen, so it will have to be aligned in some position, in the case of QGraphicsView the alignment property is used

still unclear to me how these concepts work - eg:
* setMap() and centerOn() calling mapToScene(viewport()->rect()).boundingRect());
  _map->setOutputProjection(_outputProjection); # PCS::pcs(3857);  # from pcs.h
	_map->setInputProjection(_inputProjection); #  GCS::gcs(4326);
  connect(_map, &Map::tilesLoaded, this, &MapView::reloadMap);
  _map->zoomFit(viewport()->rect().size(), cr);
	_scene->setSceneRect(_map->bounds());
* rescale() calling _sceene->setSceneRect(_map->bounds())
* paintEvent() calling mapToScene(some weird math)

why the only call to Map::draw is in MapView::drawBackground ? built-in QGraphicsView override pattern :)
because QGraphicsView::drawBackground is called whenever something of the graphics view needs to be redrawn. This something might be smaller than the whole view for a better performance

the interaction with Maps is indirect eg reloadMap calls _scene.invalidate()

now, maps (inherits QObject) eg MBTilesMap

draw(QPainter, QRect, Flags)
calls drawTile(QPainter, QPixmap, QPointF)
... which scales the pixmap with pixmap.setDevicePixelRatio(imageRatio()); and calls painter->drawPixmap

GPXSee already knows how to do transparency, see the Map settings UI, and MapView::drawBackground which simply uses painter->setOpacity !
now I just need to use QPainter.setCompositionMod(QPainter::CompositionMode_Multiply=13)
Warning: Only a QPainter operating on a QImage fully supports all composition modes

unfortunately we use QPixmap (
  * The QPixmap class is an off-screen image representation that can be used as a paint device.
  * The QImage class provides a hardware-independent image representation that allows direct access to the pixel data, and can be used as a paint device.
short summary that usually (not always) applies:
* If you plan to manipulate an image, modify it, change pixels on it, etc., use a QImage.
* If you plan to draw the same image more than once on the screen, convert it to a QPixmap.



so the plan is (ssumign I can ignore the Warning above!!!)

have an array of _mask in addition to _map
draw all of them in drawBackground
profit!

)
