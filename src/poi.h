#ifndef POI_H
#define POI_H

#include <QVector>
#include <QPointF>
#include <QString>
#include "waypoint.h"
#include "rtree.h"


class POI
{
public:
	POI() : _errorLine(0) {}
	bool loadFile(const QString &fileName);
	QString errorString() const {return _error;}
	int errorLine() const {return _errorLine;}

	QVector<Waypoint> points(const QVector<QPointF> &path,
	  qreal radius = 0.01) const;

	void clear();

private:
	typedef RTree<size_t, qreal, 2> POITree;

	bool loadCSVFile(const QString &fileName);
	bool loadGPXFile(const QString &fileName);

	POITree _tree;
	QVector<Waypoint> _data;

	QString _error;
	int _errorLine;
};

#endif // POI_H
