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
#include "data.h"


Data::Data() : _errorLine(0)
{
	_parsers.insert("gpx", new GPXParser(_trackData, _routeData,
	  _waypointData));
	_parsers.insert("tcx", new TCXParser(_trackData, _routeData,
	  _waypointData));
	_parsers.insert("kml", new KMLParser(_trackData, _routeData,
	  _waypointData));
	_parsers.insert("fit", new FITParser(_trackData, _routeData,
	  _waypointData));
	_parsers.insert("csv", new CSVParser(_trackData, _routeData,
	  _waypointData));
	_parsers.insert("igc", new IGCParser(_trackData, _routeData,
	  _waypointData));
	_parsers.insert("nmea", new NMEAParser(_trackData, _routeData,
	  _waypointData));
}

Data::~Data()
{
	QHash<QString, Parser*>::iterator it;

	for (it = _parsers.begin(); it != _parsers.end(); it++)
		delete it.value();

	for (int i = 0; i < _tracks.count(); i++)
		delete _tracks.at(i);
	for (int i = 0; i < _routes.count(); i++)
		delete _routes.at(i);
}

void Data::createData()
{
	for (int i = 0; i < _trackData.count(); i++)
		_tracks.append(new Track(_trackData.at(i)));
	for (int i = 0; i < _routeData.count(); i++)
		_routes.append(new Route(_routeData.at(i)));
}

bool Data::loadFile(const QString &fileName)
{
	QFile file(fileName);
	QFileInfo fi(fileName);


	_errorString.clear();
	_errorLine = 0;

	if (!file.open(QFile::ReadOnly)) {
		_errorString = qPrintable(file.errorString());
		return false;
	}

	QHash<QString, Parser*>::iterator it;
	if ((it = _parsers.find(fi.suffix().toLower())) != _parsers.end()) {
		if (it.value()->loadFile(&file)) {
			createData();
			return true;
		}

		_errorLine = it.value()->errorLine();
		_errorString = it.value()->errorString();
	} else {
		for (it = _parsers.begin(); it != _parsers.end(); it++) {
			if (it.value()->loadFile(&file)) {
				createData();
				return true;
			}
			file.reset();
		}

		qWarning("Error loading data file: %s:\n", qPrintable(fileName));
		for (it = _parsers.begin(); it != _parsers.end(); it++)
			qWarning("%s: line %d: %s\n", qPrintable(it.key()),
			  it.value()->errorLine(), qPrintable(it.value()->errorString()));

		_errorLine = 0;
		_errorString = "Unknown format";
	}

	return false;
}
