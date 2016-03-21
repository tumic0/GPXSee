#include "parser.h"


void Parser::handleExtensionData(QStringRef element, const QString &value)
{
	if (element == "speed")
		_track->last().speed = value.toDouble();
	else if (element == "hr" || element == "heartrate")
		_track->last().heartRate = value.toDouble();
}

void Parser::handleTrekPointData(QStringRef element, const QString &value)
{
	if (element == "ele")
		_track->last().elevation = value.toLatin1().toDouble();
	else if (element == "time")
		_track->last().timestamp = QDateTime::fromString(value.toLatin1(),
		  Qt::ISODate);
	else if (element == "geoidheight")
		_track->last().geoidheight = value.toLatin1().toDouble();
}

void Parser::handleWayPointData(QStringRef element, const QString &value)
{
	if (element == "name")
		_waypoints.last().setDescription(value);
}

void Parser::handleTrekPointAttributes(const QXmlStreamAttributes &attr)
{
	_track->last().coordinates.setY(attr.value("lat").toLatin1().toDouble());
	_track->last().coordinates.setX(attr.value("lon").toLatin1().toDouble());
}

void Parser::handleWayPointAttributes(const QXmlStreamAttributes &attr)
{
	_waypoints.last().setCoordinates(QPointF(
	  attr.value("lon").toLatin1().toDouble(),
	  attr.value("lat").toLatin1().toDouble()));
}

void Parser::extensions()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed" || _reader.name() == "hr"
		  || _reader.name() == "heartrate")
			handleExtensionData(_reader.name(), _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void Parser::trackPointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "ele" || _reader.name() == "time"
		  || _reader.name() == "geoidheight")
			handleTrekPointData(_reader.name(), _reader.readElementText());
		else if (_reader.name() == "extensions")
			extensions();
		else
			_reader.skipCurrentElement();
	}
}

void Parser::trackPoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkpt") {
			_track->append(Trackpoint());
			handleTrekPointAttributes(_reader.attributes());
			trackPointData();
		} else
			_reader.skipCurrentElement();
	}
}

void Parser::track()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkseg") {
			trackPoints();
		} else
			_reader.skipCurrentElement();
	}
}

void Parser::wayPointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			handleWayPointData(_reader.name(), _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void Parser::gpx()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trk") {
			_tracks.append(QVector<Trackpoint>());
			_track = &_tracks.back();
			track();
		} else if (_reader.name() == "wpt") {
			_waypoints.append(Waypoint());
			handleWayPointAttributes(_reader.attributes());
			wayPointData();
		} else
			_reader.skipCurrentElement();
	}
}

bool Parser::parse()
{
	if (_reader.readNextStartElement()) {
		if (_reader.name() == "gpx")
			gpx();
		else
			_reader.raiseError("Not a GPX file.");
	}

	return !_reader.error();
}

bool Parser::loadFile(QIODevice *device)
{
	_reader.clear();
	_reader.setDevice(device);

	return parse();
}
