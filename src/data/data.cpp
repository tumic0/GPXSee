#include <QApplication>
#include <QFile>
#include <QFileInfo>
#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif // Q_OS_ANDROID
#include "common/util.h"
#include "map/crs.h"
#include "gpxparser.h"
#include "tcxparser.h"
#include "csvparser.h"
#include "kmlparser.h"
#include "fitparser.h"
#include "igcparser.h"
#include "nmeaparser.h"
#include "oziparsers.h"
#include "locparser.h"
#include "slfparser.h"
#include "geojsonparser.h"
#include "exifparser.h"
#include "cupparser.h"
#include "gpiparser.h"
#include "smlparser.h"
#include "ov2parser.h"
#include "itnparser.h"
#include "onmoveparsers.h"
#include "twonavparser.h"
#include "gpsdumpparser.h"
#include "txtparser.h"
#include "vtkparser.h"
#include "vkxparser.h"
#include "mp4parser.h"
#include "srtparser.h"
#include "data.h"

#define PERMISSION "android.permission.ACCESS_MEDIA_LOCATION"

static GPXParser gpx;
static TCXParser tcx;
static KMLParser kml;
static FITParser fit;
static CSVParser csv;
static IGCParser igc;
static NMEAParser nmea;
static PLTParser plt;
static WPTParser wpt;
static RTEParser rte;
static LOCParser loc;
static SLFParser slf;
static GeoJSONParser geojson;
static EXIFParser exif;
static CUPParser cup;
static GPIParser gpi;
static SMLParser sml;
static OV2Parser ov2;
static ITNParser itn;
static OMDParser omd;
static GHPParser ghp;
static TwoNavParser twonav;
static GPSDumpParser gpsdump;
static TXTParser txt;
static VTKParser vtk;
static VKXParser vkx;
static MP4Parser mp4;
static SRTParser srt;

static QMultiMap<QString, Parser*> parsers()
{
	QMultiMap<QString, Parser*> map;

	map.insert("gpx", &gpx);
	map.insert("tcx", &tcx);
	map.insert("kml", &kml);
	map.insert("kmz", &kml);
	map.insert("fit", &fit);
	map.insert("csv", &csv);
	map.insert("igc", &igc);
	map.insert("nmea", &nmea);
	map.insert("plt", &plt);
	map.insert("wpt", &wpt);
	map.insert("rte", &rte);
	map.insert("loc", &loc);
	map.insert("slf", &slf);
	map.insert("json", &geojson);
	map.insert("geojson", &geojson);
	map.insert("jpeg", &exif);
	map.insert("jpg", &exif);
	map.insert("cup", &cup);
	map.insert("gpi", &gpi);
	map.insert("sml", &sml);
	map.insert("ov2", &ov2);
	map.insert("itn", &itn);
	map.insert("omd", &omd);
	map.insert("ghp", &ghp);
	map.insert("trk", &twonav);
	map.insert("rte", &twonav);
	map.insert("wpt", &twonav);
	map.insert("wpt", &gpsdump);
	map.insert("txt", &txt);
	map.insert("vtk", &vtk);
	map.insert("vkx", &vkx);
	map.insert("mp4", &mp4);
	map.insert("mov", &mp4);
	map.insert("raw", &mp4);
	map.insert("srt", &srt);

	return map;
}

QMultiMap<QString, Parser*> Data::_parsers = parsers();
#ifdef Q_OS_ANDROID
bool Data::_permChecked = false;
#endif // Q_OS_ANDROID

void Data::processData(QList<TrackData> &trackData, QList<RouteData> &routeData)
{
	for (int i = 0; i < trackData.count(); i++)
		_tracks.append(Track(trackData.at(i)));
	for (int i = 0; i < routeData.count(); i++)
		_routes.append(Route(routeData.at(i)));
}

#ifdef Q_OS_ANDROID
static bool isMedia(const QString &suffix)
{
	return (suffix == "jpg" || suffix == "jpeg" || suffix == "mp4"
	  || suffix == "mov");
}
#endif // Q_OS_ANDROID

Data::Data(const QString &fileName, bool tryUnknown)
{
	QFile file(fileName);
	QFileInfo fi(Util::displayName(fileName));
	QString suffix(fi.suffix().toLower());
	QList<TrackData> trackData;
	QList<RouteData> routeData;

	_valid = false;
	_errorLine = 0;

#ifdef Q_OS_ANDROID
	if (!_permChecked && isMedia(suffix)) {
		QFuture<QtAndroidPrivate::PermissionResult> res;
		res = QtAndroidPrivate::checkPermission(PERMISSION);
		if (res.result() != QtAndroidPrivate::Authorized) {
			res = QtAndroidPrivate::requestPermission(PERMISSION);
			if (res.result() != QtAndroidPrivate::Authorized)
				qWarning("Error getting ACCESS_MEDIA_LOCATION permissions");
		}
		_permChecked = true;
	}
#endif // Q_OS_ANDROID

	if (!file.open(QFile::ReadOnly)) {
		_errorString = file.errorString();
		return;
	}

	QMultiMap<QString, Parser*>::iterator it;
	if ((it = _parsers.find(suffix)) != _parsers.end()) {
		while (it != _parsers.end() && it.key() == suffix) {
			if (it.value()->parse(&file, trackData, routeData, _polygons,
			  _waypoints)) {
				processData(trackData, routeData);
				_valid = true;
				return;
			} else {
				_errorLine = it.value()->errorLine();
				_errorString = it.value()->errorString();
			}
			file.reset();
			++it;
		}

		qWarning("%s:", qUtf8Printable(fileName));
		for (it = _parsers.find(suffix); it != _parsers.end()
		  && it.key() == suffix; it++)
			qWarning("  %s: line %d: %s", qUtf8Printable(it.key()),
			  it.value()->errorLine(), qUtf8Printable(it.value()->errorString()));

	} else if (tryUnknown) {
		for (it = _parsers.begin(); it != _parsers.end(); it++) {
			if (it.value()->parse(&file, trackData, routeData, _polygons,
			  _waypoints)) {
				processData(trackData, routeData);
				_valid = true;
				return;
			}
			file.reset();
		}

		qWarning("%s:", qUtf8Printable(fileName));
		for (it = _parsers.begin(); it != _parsers.end(); it++)
			qWarning("  %s: line %d: %s", qUtf8Printable(it.key()),
			  it.value()->errorLine(), qUtf8Printable(it.value()->errorString()));

		_errorLine = 0;
		_errorString = "Unknown format";
	}
}

Data::Data(const QUrl &url)
{
	bool caOk, cbOk, ccOk;
	Projection proj(GCS::WGS84());

	_valid = false;

	QStringList parts(url.path().split(';'));
	if (parts.size() < 1) {
		_errorString = "Syntax error";
		return;
	}
	QStringList coords(parts.at(0).split(','));
	if (coords.size() < 2 || coords.size() > 3) {
		_errorString = "Syntax error";
		return;
	}
	double ca = coords.at(0).toDouble(&caOk);
	double cb = coords.at(1).toDouble(&cbOk);
	double cc = NAN;
	if (!(caOk && cbOk)) {
		_errorString = "Invalid coordinates";
		return;
	}
	if (coords.size() > 2) {
		cc = coords.at(2).toDouble(&ccOk);
		if (!ccOk) {
			_errorString = "Invalid elevation";
			return;
		}
	}

	if (parts.size() > 1) {
		QStringList crsp(parts.at(1).split('='));
		if (crsp.size() != 2) {
			_errorString = "Syntax error";
			return;
		}
		if (!crsp.at(0).compare("crs", Qt::CaseInsensitive)) {
			if (crsp.at(1).compare("wgs84", Qt::CaseInsensitive)) {
				proj = CRS::projection(crsp.at(1));
				if (!proj.isValid()) {
					_errorString = "Unknown CRS";
					return;
				}
			}
		}
	}

	CoordinateSystem::AxisOrder ao = proj.coordinateSystem().axisOrder();
	PointD p(ao == CoordinateSystem::XY ? PointD(ca, cb) : PointD(cb, ca));
	Coordinates c(proj.xy2ll(p));
	if (!c.isValid()) {
		_errorString = "Invalid coordinates";
		return;
	}

	Waypoint w(c);
	w.setElevation(cc);
	_waypoints.append(w);

	_valid = true;
}

QString Data::formats()
{
	return
	  QCoreApplication::translate("Data", "Supported files")
	    + " (" + filter().join(" ") + ");;"
	  + QCoreApplication::translate("Data", "CSV files") + " (*.csv);;"
	  + QCoreApplication::translate("Data", "CUP files") + " (*.cup);;"
	  + QCoreApplication::translate("Data", "FIT files") + " (*.fit);;"
	  + QCoreApplication::translate("Data", "GeoJSON files")
	    + " (*.geojson *.json);;"
	  + QCoreApplication::translate("Data", "GPI files") + " (*.gpi);;"
	  + QCoreApplication::translate("Data", "GPX files") + " (*.gpx);;"
	  + QCoreApplication::translate("Data", "IGC files") + " (*.igc);;"
	  + QCoreApplication::translate("Data", "ITN files") + " (*.itn);;"
	  + QCoreApplication::translate("Data", "JPEG images") + " (*.jpg *.jpeg);;"
	  + QCoreApplication::translate("Data", "KML files") + " (*.kml *.kmz);;"
	  + QCoreApplication::translate("Data", "LOC files") + " (*.loc);;"
	  + QCoreApplication::translate("Data", "MP4 videos") + " (*.mp4 *.mov);;"
	  + QCoreApplication::translate("Data", "NMEA files") + " (*.nmea);;"
	  + QCoreApplication::translate("Data", "ONmove files") + " (*.omd *.ghp);;"
	  + QCoreApplication::translate("Data", "OV2 files") + " (*.ov2);;"
	  + QCoreApplication::translate("Data", "OziExplorer files")
	    + " (*.plt *.rte *.wpt);;"
	  + QCoreApplication::translate("Data", "GPMF files") + " (*.raw);;"
	  + QCoreApplication::translate("Data", "SLF files") + " (*.slf);;"
	  + QCoreApplication::translate("Data", "SML files") + " (*.sml);;"
	  + QCoreApplication::translate("Data", "DJI SRT files") + " (*.srt);;"
	  + QCoreApplication::translate("Data", "TCX files") + " (*.tcx);;"
	  + QCoreApplication::translate("Data", "70mai GPS log files") + " (*.txt);;"
	  + QCoreApplication::translate("Data", "VKX files") + " (*.vkx);;"
	  + QCoreApplication::translate("Data", "VTK files") + " (*.vtk);;"
	  + QCoreApplication::translate("Data", "TwoNav files")
	    + " (*.rte *.trk *.wpt);;"
	  + QCoreApplication::translate("Data", "GPSDump files") + " (*.wpt);;"
	  + QCoreApplication::translate("Data", "All files") + " (*)";
}

QStringList Data::filter()
{
	QStringList filter;
	QString last;

	for (QMultiMap<QString, Parser*>::iterator it = _parsers.begin();
	  it != _parsers.end(); it++) {
		if (it.key() != last)
			filter << "*." + it.key();
		last = it.key();
	}

	return filter;
}
