#include "parser.h"


void Parser::handleDataElement(Element element, const QString &value)
{
	switch (element) {
	    case Elevation:
		    _track->last().elevation = value.toLatin1().toDouble();
		    break;
	    case Time:
		    _track->last().timestamp = QDateTime::fromString(value.toLatin1(),
			  Qt::ISODate);
		    break;
	    case Geoidheight:
		    _track->last().geoidheight = value.toLatin1().toDouble();
		    break;
	    case Speed:
		    _track->last().speed = value.toDouble();
		    break;
	    case HeartRate:
		    _track->last().heartRate = value.toDouble();
		    break;
	    case Temperature:
		    _track->last().temperature = value.toDouble();
		    break;
	}
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

void Parser::tpExtension()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "hr")
			handleDataElement(HeartRate, _reader.readElementText());
		else if (_reader.name() == "atemp")
			handleDataElement(Temperature, _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void Parser::extensions()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed")
			handleDataElement(Speed, _reader.readElementText());
		else if (_reader.name() == "hr" || _reader.name() == "heartrate")
			handleDataElement(HeartRate, _reader.readElementText());
		else if (_reader.name() == "temp")
			handleDataElement(Temperature, _reader.readElementText());
		else if (_reader.name() == "TrackPointExtension")
			tpExtension();
		else
			_reader.skipCurrentElement();
	}
}

void Parser::trackPointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "ele")
			handleDataElement(Elevation, _reader.readElementText());
		else if (_reader.name() == "time")
			handleDataElement(Time, _reader.readElementText());
		else if (_reader.name() == "geoidheight")
			handleDataElement(Geoidheight, _reader.readElementText());
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
