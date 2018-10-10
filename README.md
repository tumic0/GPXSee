# GPXSee
GPXSee is a Qt-based GPS log file viewer and analyzer that supports GPX, TCX,
KML, FIT, IGC, NMEA, SLF, LOC and OziExplorer files.

## Features
* User-definable online maps (OpenStreetMap/Google tiles, WMTS, WMS, TMS).
* Offline maps (MBTiles, OziExplorer maps, TrekBuddy maps/atlases, Garmin JNX maps, GeoTIFF images).
* Elevation, speed, heart rate, cadence, power, temperature and gear ratio/shifts graphs.
* Support for multiple tracks in one view.
* Support for POI files.
* Print/export to PDF.
* Full-screen mode.
* HiDPI/Retina displays & maps support.
* Native GUI for Windows, Mac OS X and Linux.
* Opens GPX, TCX, FIT, KML, IGC, NMEA, SLF, LOC, OziExplorer (PLT, RTE, WPT) and Garmin CSV files.

![GPXSee - Linux](https://a.fsdn.com/con/app/proj/gpxsee/screenshots/linux2.png)

## Build
Build requirements:
* Qt 4.8 or QT 5.x (Qt >= 5.10.1 recommended for all features)
* C++03 compiler (tested: msvc2015, gcc >= 4.8, clang/Apple LLVM version 8.1.0)

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
http://www.gpxsee.org

## Maps
[GPXSee maps repository](https://github.com/tumic0/GPXSee-maps)

## Translations
GPXSee uses [Weblate](https://hosted.weblate.org/projects/gpxsee/translations/) for translations.
