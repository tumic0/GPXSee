#include "parser.h"


void Parser::handleExtensionData(QStringRef element, const QString &value)
{
	if (element == "speed")
		_track->last().speed = value.toDouble();
}

void Parser::handleTrekPointData(QStringRef element, const QString &value)
{
	if (element == "ele")
		_track->last().elevation = value.toLatin1().toDouble();
	if (element == "time")
		_track->last().timestamp = QDateTime::fromString(value.toLatin1(),
		  Qt::ISODate);
	if (element == "geoidheight")
		_track->last().geoidheight = value.toLatin1().toDouble();
}

void Parser::handleTrekPointAttributes(const QXmlStreamAttributes &attr)
{
	_track->last().coordinates.setY(attr.value("lat").toLatin1().toDouble());
	_track->last().coordinates.setX(attr.value("lon").toLatin1().toDouble());
}


void Parser::extensions()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed")
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
	QXmlStreamAttributes attr;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkpt") {
			attr = _reader.attributes();
			_track->append(TrackPoint());
			handleTrekPointAttributes(attr);
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

void Parser::gpx()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trk") {
			_data->append(QVector<TrackPoint>());
			_track = &_data->back();
			track();
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

bool Parser::loadFile(QIODevice *device, QList<QVector<TrackPoint> > *data)
{
	_reader.clear();
	_reader.setDevice(device);
	_data = data;

	return parse();
}
