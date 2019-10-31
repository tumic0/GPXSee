# GPXSee
GPXSee is a Qt-based GPS log file viewer and analyzer that supports all common GPS log file formats.

## Features
* Opens GPX, TCX, FIT, KML, NMEA, IGC, CUP, SLF, LOC, GeoJSON, OziExplorer (PLT, RTE, WPT), Garmin GPI&CSV and geotagged JPEG files.
* User-definable online maps (OpenStreetMap/Google tiles, WMTS, WMS, TMS, QuadTiles).
* Offline maps (MBTiles, OziExplorer maps, TrekBuddy maps/atlases, Garmin IMG & JNX maps, TwoNav RMaps, GeoTIFF images).
* Elevation, speed, heart rate, cadence, power, temperature and gear ratio/shifts graphs.
* Support for DEM files (SRTM HGT).
* Support for multiple tracks in one view.
* Support for POI files.
* Print/export to PDF.
* Full-screen mode.
* HiDPI/Retina displays & maps support.
* Native GUI for Windows, Mac OS X and Linux.

![GPXSee - Linux](https://a.fsdn.com/con/app/proj/gpxsee/screenshots/linux2.png)

## Build
Build requirements:
* Qt 4.8 or QT 5.x (Qt >= 5.10.1 recommended for all features)
* C++03 or newer compiler (tested: msvc2015, gcc >= 4.8, clang/Apple LLVM version 8.1.0)

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
