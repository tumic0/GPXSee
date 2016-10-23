#include <QFile>
#include <QLineF>
#include "ll.h"
#include "gpxparser.h"
#include "tcxparser.h"
#include "csvparser.h"
#include "data.h"


Data::Data() : _errorLine(0)
{
	_parsers << new GPXParser(_track_data, _route_data, _waypoint_data);
	_parsers << new TCXParser(_track_data, _route_data, _waypoint_data);
	_parsers << new CSVParser(_track_data, _route_data, _waypoint_data);
}

Data::~Data()
{
	for(int i = 0; i < _parsers.count(); i++)
		delete _parsers.at(i);

	for (int i = 0; i < _tracks.count(); i++)
		delete _tracks.at(i);
	for (int i = 0; i < _routes.count(); i++)
		delete _routes.at(i);
}

bool Data::loadFile(const QString &fileName)
{
	QFile file(fileName);

	_errorString.clear();
	_errorLine = 0;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = qPrintable(file.errorString());
		return false;
	}

	for (int i = 0; i < _parsers.size(); i++) {
		if (_parsers.at(i)->loadFile(&file)) {
			for (int i = 0; i < _track_data.count(); i++)
				_tracks.append(new Track(_track_data.at(i)));
			for (int i = 0; i < _route_data.count(); i++)
				_routes.append(new Route(_route_data.at(i)));
			return true;
		}
		file.reset();
	}

	fprintf(stderr, "Error loading data file: %s:\n", qPrintable(fileName));
	for (int i = 0; i < _parsers.size(); i++) {
		fprintf(stderr, "%s: line %d: %s\n", _parsers.at(i)->name(),
		  _parsers.at(i)->errorLine(), qPrintable(_parsers.at(i)->errorString()));
		if (_parsers.at(i)->errorLine() > _errorLine) {
			_errorLine = _parsers.at(i)->errorLine();
			_errorString = _parsers.at(i)->errorString();
		}
	}

	return false;
}
