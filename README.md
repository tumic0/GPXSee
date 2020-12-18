# GPXSee
GPXSee is a Qt-based GPS log file viewer and analyzer that supports all common GPS log file formats.

## Features
* Opens GPX, TCX, FIT, KML, NMEA, IGC, CUP, SIGMA SLF, Suunto SML, LOC, GeoJSON, OziExplorer (PLT, RTE, WPT), Garmin GPI&CSV and geotagged JPEG files.
* User-definable online maps (OpenStreetMap/Google tiles, WMTS, WMS, TMS, QuadTiles).
* Offline maps (MBTiles, OziExplorer maps, TrekBuddy maps/atlases, Garmin IMG/GMAP & JNX maps, TwoNav RMaps, GeoTIFF images, BSB charts).
* Elevation, speed, heart rate, cadence, power, temperature and gear ratio/shifts graphs.
* Support for DEM files (SRTM HGT).
* Support for multiple tracks in one view.
* Support for POI files.
* Print/export to PDF/PNG.
* Full-screen mode.
* HiDPI/Retina displays & maps support.
* Native GUI for Windows, Mac OS X and Linux.

![GPXSee - Linux](https://a.fsdn.com/con/app/proj/gpxsee/screenshots/linux2.png)

## Build
Build requirements:
* Qt 5.12 or QT 6.x
* C++11 or newer compiler (tested: msvc2017, gcc 7.5.0, clang/Apple LLVM version 10.0.0)

Build steps:
```shell
lrelease gpxsee.pro
qmake gpxsee.pro
make # nmake on windows
```

## Download
* [Windows & OS X builds](http://sourceforge.net/projects/gpxsee)
* [Linux packages](http://software.opensuse.org/download.html?project=home%3Atumic%3AGPXSee&package=gpxsee)

## Changelog
[Changelog](https://build.opensuse.org/package/view_file/home:tumic:GPXSee/gpxsee/gpxsee.changes)

## Homepage
[https://www.gpxsee.org](https://www.gpxsee.org)

## Maps
[GPXSee maps repository](https://github.com/tumic0/GPXSee-maps)

## Translations
GPXSee uses [Weblate](https://hosted.weblate.org/projects/gpxsee/translations/) for translations.

## License
GPXSee is licensed under GPL-3.0 (only). However, some 3rd party parts are using different, GPL compatible,
licenses:
* [Oxygen icons](icons/GUI) - LGPLv3
* [Mapbox Maki icons](icons/POI) - CC0
* [RTree implementation](src/common/rtree.h) - Public domain
* [Albers](src/map/albersequal.cpp), [Geocentric](src/map/geocentric.cpp), [LCC](src/map/lambertconic.cpp),
  [Mercator](src/map/mercator.cpp), [Polar Stereographic](src/map/polarstereographic.cpp),
  [Polyconic](src/map/polyconic.cpp) and [Transverse Mercator](src/map/transversemercator.cpp)
  projections - NIMA Source Code Disclaimer
* [Projection parameters CSV files](pkg/csv) - BSD/EPSG/Public domain
