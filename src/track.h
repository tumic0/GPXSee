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

	Path path() const;

	Graph elevation() const;
	Graph speed() const;
	Graph heartRate() const;
	Graph temperature() const;
	Graph cadence() const;
	Graph power() const;

	qreal distance() const;
	qreal time() const;
	qreal movingTime() const;
	QDateTime date() const;

	const QString &name() const {return _data.name();}
	const QString &description() const {return _data.description();}

	bool isNull() const {return (_data.size() < 2);}

private:
	bool discardStopPoint(int i) const;

	const TrackData &_data;

	QVector<qreal> _distance;
	QVector<qreal> _time;
	QVector<qreal> _speed;

	QSet<int> _outliers;
	QSet<int> _stop;

	qreal _pause;
};

#endif // TRACK_H
