#ifndef TRACK_H
#define TRACK_H

#include <QVector>
#include <QSet>
#include <QDateTime>
#include "trackdata.h"
#include "graph.h"
#include "path.h"


class Track
{
public:
	Track(const TrackData &data);

	Path track() const;
	Graph elevation() const;
	Graph speed() const;
	Graph heartRate() const;
	Graph temperature() const;

	qreal distance() const;
	qreal time() const;
	QDateTime date() const;

	const QString &name() const {return _data.name();}
	const QString &description() const {return _data.description();}

	bool isNull() const {return (_data.size() < 2);}

private:
	const TrackData &_data;

	QVector<qreal> _distance;
	QVector<qreal> _time;
	QVector<qreal> _speed;

	QSet<int> _outliers;
};

#endif // TRACK_H
