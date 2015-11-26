#ifndef POI_H
#define POI_H

#include <QVector>
#include <QPointF>
#include <QString>
#include "rtree.h"

class Entry
{
public:
	QPointF coordinates;
	QString description;

	bool operator==(const Entry &other) const
	  {return this->description == other.description
	  && this->coordinates == other.coordinates;}
};

inline uint qHash(const Entry &key)
{
	return ::qHash(key.description);
}


class POI
{
public:
	bool loadFile(const QString &fileName);
	QString errorString() const {return _error;}
	int errorLine() const {return _errorLine;}

	QVector<Entry> points(const QVector<QPointF> &path) const;

	void clear();

private:
	typedef RTree<size_t, qreal, 2> POITree;

	POITree _tree;
	QVector<Entry> _data;
	QString _error;
	int _errorLine;
};

#endif // POI_H
