#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include "common/util.h"
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
#include "data.h"


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

	return map;
}

QMultiMap<QString, Parser*> Data::_parsers = parsers();

void Data::processData(QList<TrackData> &trackData, QList<RouteData> &routeData)
{
	for (int i = 0; i < trackData.count(); i++)
		_tracks.append(Track(trackData.at(i)));
	for (int i = 0; i < routeData.count(); i++)
		_routes.append(Route(routeData.at(i)));
}

Data::Data(const QString &fileName, bool tryUnknown)
{
	QFile file(fileName);
	QFileInfo fi(Util::displayName(fileName));
	QList<TrackData> trackData;
	QList<RouteData> routeData;

	_valid = false;
	_errorLine = 0;

	if (!file.open(QFile::ReadOnly)) {
		_errorString = file.errorString();
		return;
	}

	QMultiMap<QString, Parser*>::iterator it;
	QString suffix(fi.suffix().toLower());
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

QString Data::formats()
{
	return
	  qApp->translate("Data", "Supported files") + " (" + filter().join(" ") + ");;"
	  + qApp->translate("Data", "CSV files") + " (*.csv);;"
	  + qApp->translate("Data", "CUP files") + " (*.cup);;"
	  + qApp->translate("Data", "FIT files") + " (*.fit);;"
	  + qApp->translate("Data", "GeoJSON files") + " (*.geojson *.json);;"
	  + qApp->translate("Data", "GPI files") + " (*.gpi);;"
	  + qApp->translate("Data", "GPX files") + " (*.gpx);;"
	  + qApp->translate("Data", "IGC files") + " (*.igc);;"
	  + qApp->translate("Data", "ITN files") + " (*.itn);;"
	  + qApp->translate("Data", "JPEG images") + " (*.jpg *.jpeg);;"
	  + qApp->translate("Data", "KML files") + " (*.kml *.kmz);;"
	  + qApp->translate("Data", "LOC files") + " (*.loc);;"
	  + qApp->translate("Data", "NMEA files") + " (*.nmea);;"
	  + qApp->translate("Data", "ONmove files") + " (*.omd *.ghp);;"
	  + qApp->translate("Data", "OV2 files") + " (*.ov2);;"
	  + qApp->translate("Data", "OziExplorer files") + " (*.plt *.rte *.wpt);;"
	  + qApp->translate("Data", "SLF files") + " (*.slf);;"
	  + qApp->translate("Data", "SML files") + " (*.sml);;"
	  + qApp->translate("Data", "TCX files") + " (*.tcx);;"
	  + qApp->translate("Data", "TwoNav files") + " (*.rte *.trk *.wpt);;"
	  + qApp->translate("Data", "GPSDump files") + " (*.wpt);;"
	  + qApp->translate("Data", "All files") + " (*)";
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
