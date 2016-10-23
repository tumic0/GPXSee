#ifndef POI_H
#define POI_H

#include <QVector>
#include <QPointF>
#include <QString>
#include <QStringList>
#include "waypoint.h"
#include "rtree.h"

class PathItem;
class WaypointItem;

class POI : public QObject
{
	Q_OBJECT

public:
	POI(QObject *parent = 0);

	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

	qreal radius() const {return _radius;}
	void setRadius(qreal radius);

	QVector<Waypoint> points(const PathItem *path) const;
	QVector<Waypoint> points(const QList<WaypointItem*> &list) const;
	QVector<Waypoint> points(const QList<Waypoint> &list) const;

	const QStringList &files() const {return _files;}
	void enableFile(const QString &fileName, bool enable);
	void clear();

signals:
	void pointsChanged();

private:
	typedef RTree<size_t, qreal, 2> POITree;
	typedef struct {
		int start;
		int end;
		bool enabled;
	} FileIndex;

	POITree _tree;
	QVector<Waypoint> _data;
	QStringList _files;
	QList<FileIndex> _indexes;

	qreal _radius;

	QString _errorString;
	int _errorLine;
};

#endif // POI_H
