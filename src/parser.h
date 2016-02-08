#ifndef PARSER_H
#define PARSER_H

#include <QXmlStreamReader>
#include <QDateTime>
#include <QPointF>

struct TrackPoint
{
	QPointF coordinates;
	QDateTime timestamp;
	qreal elevation;
	qreal geoidheight;
	qreal speed;

	TrackPoint() {elevation = 0; geoidheight = 0; speed = -1;}
};

class Parser
{
public:
	Parser() {_data = 0; _track = 0;}
	bool loadFile(QIODevice *device, QList<QVector<TrackPoint> > *data);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	bool parse();
	void gpx();
	void track();
	void trackPoints();
	void extensions();
	void trackPointData();

	void handleTrekPointAttributes(const QXmlStreamAttributes &attr);
	void handleTrekPointData(QStringRef element, const QString &value);
	void handleExtensionData(QStringRef element, const QString &value);

	QXmlStreamReader _reader;
	QList<QVector<TrackPoint> > *_data;
	QVector<TrackPoint> *_track;
};

#endif // PARSER_H
