#include "parser.h"

#include <QDebug>


void Parser::handleExtensionData(QVector<TrackPoint> &data,
  QStringRef element, const QString &value)
{
	if (element == "speed")
		data.last().speed = value.toDouble();
}

void Parser::handleTrekPointData(QVector<TrackPoint> &data,
  QStringRef element, const QString &value)
{
	if (element == "ele")
		data.last().elevation = value.toLatin1().toDouble();
	if (element == "time")
		data.last().timestamp = QDateTime::fromString(value.toLatin1(),
		  Qt::ISODate);
}

void Parser::handleTrekPointAttributes(QVector<TrackPoint> &data,
  const QXmlStreamAttributes &attr)
{
	data.last().coordinates.setY(attr.value("lat").toLatin1().toDouble());
	data.last().coordinates.setX(attr.value("lon").toLatin1().toDouble());
}


void Parser::extensions(QVector<TrackPoint> &data)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed")
			handleExtensionData(data, _reader.name(), _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void Parser::trekPointData(QVector<TrackPoint> &data)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "ele" || _reader.name() == "time")
			handleTrekPointData(data, _reader.name(), _reader.readElementText());
		else if (_reader.name() == "extensions")
			extensions(data);
		else
			_reader.skipCurrentElement();
	}
}

void Parser::trekPoints(QVector<TrackPoint> &data)
{
	QXmlStreamAttributes attr;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkpt") {
			attr = _reader.attributes();
			data.append(TrackPoint());
			handleTrekPointAttributes(data, attr);
			trekPointData(data);
		} else
			_reader.skipCurrentElement();
	}
}

void Parser::trek(QVector<TrackPoint> &data)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkseg") {
			trekPoints(data);
		} else
			_reader.skipCurrentElement();
	}
}

void Parser::gpx(QVector<TrackPoint> &data)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trk")
			trek(data);
		else
			_reader.skipCurrentElement();
	}
}

bool Parser::parse(QVector<TrackPoint> &data)
{
	if (_reader.readNextStartElement()) {
		if (_reader.name() == "gpx")
			gpx(data);
		else
			_reader.raiseError(QObject::tr("Not a GPX file."));
	}

	return !_reader.error();
}


QString Parser::errorString() const
{
	return QObject::tr("%1\nLine %2")
		.arg(_reader.errorString())
		.arg(_reader.lineNumber());
}

bool Parser::loadFile(QIODevice *device, QVector<TrackPoint> &data)
{
	_reader.clear();
	_reader.setDevice(device);

	return parse(data);
}
