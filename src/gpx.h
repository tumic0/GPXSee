#ifndef GPX_H
#define GPX_H

#include <QVector>
#include <QList>
#include <QPointF>
#include <QString>
#include "waypoint.h"
#include "track.h"
#include "parser.h"

class GPX
{
public:
	GPX() : _parser(_tracks, _waypoints) {}
	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _error;}
	int errorLine() const {return _errorLine;}

	int trackCount() const {return _tracks.count();}
	Track track(int i) const {return Track(_tracks.at(i));}
	const QList<Waypoint> &waypoints() const {return _waypoints;}

private:
	Parser _parser;
	QString _error;
	int _errorLine;

	QList<QVector<Trackpoint> > _tracks;
	QList<Waypoint> _waypoints;
};

#endif // GPX_H
