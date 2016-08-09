#include <QFile>
#include <QLineF>
#include "ll.h"
#include "gpx.h"


GPX::GPX() : _parser(_track_data, _route_data, _waypoint_data), _errorLine(0)
{
}

GPX::~GPX()
{
	for (int i = 0; i < _tracks.count(); i++)
		delete _tracks.at(i);
	for (int i = 0; i < _routes.count(); i++)
		delete _routes.at(i);
}

bool GPX::loadFile(const QString &fileName)
{
	bool ret;
	QFile file(fileName);


	_error.clear();
	_errorLine = 0;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_error = qPrintable(file.errorString());
		return false;
	}

	ret = _parser.loadFile(&file);
	file.close();
	if (ret == false) {
		_error = _parser.errorString();
		_errorLine = _parser.errorLine();
		return false;
	}

	for (int i = 0; i < _track_data.count(); i++)
		_tracks.append(new Track(_track_data.at(i)));
	for (int i = 0; i < _route_data.count(); i++)
		_routes.append(new Route(_route_data.at(i)));

	return true;
}
