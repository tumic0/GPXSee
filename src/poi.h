#ifndef POI_H
#define POI_H

#include <QVector>
#include <QPointF>
#include <QString>
#include <QStringList>
#include "waypoint.h"
#include "rtree.h"

class QPainterPath;
class WaypointItem;

#define POI_RADIUS 0.01

class POI
{
public:
	POI() : _errorLine(0) {}
	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _error;}
	int errorLine() const {return _errorLine;}

	QVector<Waypoint> points(const QPainterPath &path,
	  qreal radius = POI_RADIUS) const;
	QVector<Waypoint> points(const QList<WaypointItem*> &list,
	  qreal radius = POI_RADIUS) const;

	const QStringList &files() const {return _files;}
	void enableFile(const QString &fileName, bool enable);
	void clear();

private:
	typedef RTree<size_t, qreal, 2> POITree;
	typedef struct {
		int start;
		int end;
		bool enabled;
	} FileIndex;

	bool loadCSVFile(const QString &fileName);
	bool loadGPXFile(const QString &fileName);

	POITree _tree;
	QVector<Waypoint> _data;
	QStringList _files;
	QList<FileIndex> _indexes;

	QString _error;
	int _errorLine;
};

#endif // POI_H
