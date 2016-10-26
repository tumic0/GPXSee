#include <QFile>
#include <QFileInfo>
#include <QLineF>
#include "gpxparser.h"
#include "tcxparser.h"
#include "csvparser.h"
#include "kmlparser.h"
#include "data.h"


Data::Data() : _errorLine(0)
{
	_parsers.insert("gpx", new GPXParser(_track_data, _route_data,
	  _waypoint_data));
	_parsers.insert("tcx", new TCXParser(_track_data, _route_data,
	  _waypoint_data));
	_parsers.insert("kml", new KMLParser(_track_data, _route_data,
	  _waypoint_data));
	_parsers.insert("csv", new CSVParser(_track_data, _route_data,
	  _waypoint_data));
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
	for (int i = 0; i < _track_data.count(); i++)
		_tracks.append(new Track(_track_data.at(i)));
	for (int i = 0; i < _route_data.count(); i++)
		_routes.append(new Route(_route_data.at(i)));
}

bool Data::loadFile(const QString &fileName)
{
	QFile file(fileName);
	QFileInfo fi(fileName);


	_errorString.clear();
	_errorLine = 0;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
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

		fprintf(stderr, "Error loading data file: %s:\n", qPrintable(fileName));
		for (it = _parsers.begin(); it != _parsers.end(); it++)
			fprintf(stderr, "%s: line %d: %s\n", qPrintable(it.key()),
			  it.value()->errorLine(), qPrintable(it.value()->errorString()));

		_errorLine = 0;
		_errorString = "Unknown format";
	}

	return false;
}
