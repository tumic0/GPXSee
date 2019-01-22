#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QLineF>
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
#include "dem.h"
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

static QHash<QString, Parser*> parsers()
{
	QHash<QString, Parser*> hash;

	hash.insert("gpx", &gpx);
	hash.insert("tcx", &tcx);
	hash.insert("kml", &kml);
	hash.insert("fit", &fit);
	hash.insert("csv", &csv);
	hash.insert("igc", &igc);
	hash.insert("nmea", &nmea);
	hash.insert("plt", &plt);
	hash.insert("wpt", &wpt);
	hash.insert("rte", &rte);
	hash.insert("loc", &loc);
	hash.insert("slf", &slf);

	return hash;
}


QHash<QString, Parser*> Data::_parsers = parsers();
bool Data::_useDEM = false;

Data::~Data()
{
	for (int i = 0; i < _tracks.count(); i++)
		delete _tracks.at(i);
	for (int i = 0; i < _routes.count(); i++)
		delete _routes.at(i);
}

void Data::processData()
{
	for (int i = 0; i < _trackData.count(); i++)
		_tracks.append(new Track(_trackData.at(i)));
	for (int i = 0; i < _routeData.count(); i++)
		_routes.append(new Route(_routeData.at(i)));
	for (int i = 0; i < _waypoints.size(); i++) {
		if (!_waypoints.at(i).hasElevation() || _useDEM) {
			qreal elevation = DEM::elevation(_waypoints.at(i).coordinates());
			if (!std::isnan(elevation))
				_waypoints[i].setElevation(elevation);
		}
	}
}

Data::Data(const QString &fileName, bool poi)
{
	QFile file(fileName);
	QFileInfo fi(fileName);

	_valid = false;
	_errorLine = 0;

	if (!file.open(QFile::ReadOnly)) {
		_errorString = qPrintable(file.errorString());
		return;
	}

	QHash<QString, Parser*>::iterator it;
	if ((it = _parsers.find(fi.suffix().toLower())) != _parsers.end()) {
		if (it.value()->parse(&file, _trackData, _routeData, _waypoints)) {
			if (!poi)
				processData();
			_valid = true;
			return;
		} else {
			_errorLine = it.value()->errorLine();
			_errorString = it.value()->errorString();
		}
	} else {
		for (it = _parsers.begin(); it != _parsers.end(); it++) {
			if (it.value()->parse(&file, _trackData, _routeData, _waypoints)) {
				if (!poi)
					processData();
				_valid = true;
				return;
			}
			file.reset();
		}

		qWarning("Error loading data file: %s:", qPrintable(fileName));
		for (it = _parsers.begin(); it != _parsers.end(); it++)
			qWarning("%s: line %d: %s", qPrintable(it.key()),
			  it.value()->errorLine(), qPrintable(it.value()->errorString()));

		_errorLine = 0;
		_errorString = "Unknown format";
	}
}

QString Data::formats()
{
	return
	  qApp->translate("Data", "Supported files")
	  + " (*.csv *.fit *.gpx *.igc *.kml *.loc *.nmea *.plt *.rte *.slf *.tcx *.wpt);;"
	  + qApp->translate("Data", "CSV files") + " (*.csv);;"
	  + qApp->translate("Data", "FIT files") + " (*.fit);;"
	  + qApp->translate("Data", "GPX files") + " (*.gpx);;"
	  + qApp->translate("Data", "IGC files") + " (*.igc);;"
	  + qApp->translate("Data", "KML files") + " (*.kml);;"
	  + qApp->translate("Data", "LOC files") + " (*.loc);;"
	  + qApp->translate("Data", "NMEA files") + " (*.nmea);;"
	  + qApp->translate("Data", "OziExplorer files") + " (*.plt *.rte *.wpt);;"
	  + qApp->translate("Data", "SLF files") + " (*.slf);;"
	  + qApp->translate("Data", "TCX files") + " (*.tcx);;"
	  + qApp->translate("Data", "All files") + " (*)";
}

QStringList Data::filter()
{
	QStringList filter;
	QHash<QString, Parser*>::iterator it;

	for (it = _parsers.begin(); it != _parsers.end(); it++)
		filter << QString("*.%1").arg(it.key());

	return filter;
}

void Data::useDEM(bool use)
{
	_useDEM = use;
	Route::useDEM(use);
	Track::useDEM(use);
}
