# GPXSee
GPXSee is a Qt-based GPS log file viewer and analyzer that supports GPX, TCX,
KML, FIT, IGC and NMEA files.

## Features
* User-definable map sources.
* Elevation, speed, heart rate, cadence, power and temperature graphs.
* Support for multiple tracks in one view.
* Support for POI files.
* Print/export to PDF.
* Full-screen mode.
* Native GUI for Windows, Mac OS X and Linux.
* Opens GPX, TCX, FIT, KML, IGC, NMEA and Garmin CSV files.

![GPXSee - Linux](https://a.fsdn.com/con/app/proj/gpxsee/screenshots/linux.png)

## Build
### Linux/OS X
```shell
lrelease gpxsee.pro
qmake gpxsee.pro
make
```
### Windows
```shell
lrelease gpxsee.pro
qmake gpxsee.pro
nmake release
```

## Binaries
* Windows & OS X builds: http://sourceforge.net/projects/gpxsee
* Linux packages: https://build.opensuse.org/project/repositories/home:tumic:GPXSee

## Homepage
GPXSee homepage: http://tumic.wz.cz/gpxsee
