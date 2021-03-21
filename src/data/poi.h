#ifndef POI_H
#define POI_H

#include <QList>
#include <QPointF>
#include <QString>
#include <QStringList>
#include "common/rtree.h"
#include "common/treenode.h"
#include "waypoint.h"

class Path;
class RectC;

class POI : public QObject
{
	Q_OBJECT

public:
	POI(QObject *parent = 0);
	~POI();

	bool loadFile(const QString &path);
	TreeNode<QString> loadDir(const QString &path);
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

	unsigned radius() const {return _radius;}
	void setRadius(unsigned radius);

	QList<Waypoint> points(const Path &path) const;
	QList<Waypoint> points(const Waypoint &point) const;
	QList<Waypoint> points(const RectC &rect) const;

	bool isLoaded(const QString &path) const {return _files.contains(path);}
	bool enableFile(const QString &fileName, bool enable);

signals:
	void pointsChanged();

private:
	typedef RTree<size_t, qreal, 2> POITree;
	class File {
	public:
		File(int start, int end, const QVector<Waypoint> &data);

		void search(const RectC &rect, QSet<int> &set) const;
		void enable(bool enable) {_enabled = enable;}

	private:
		bool _enabled;
		POITree _tree;
	};
	typedef QHash<QString, File*>::const_iterator ConstIterator;
	typedef QHash<QString, File*>::iterator Iterator;

	void search(const RectC &rect, QSet<int> &set) const;

	QVector<Waypoint> _data;
	QHash<QString, File*> _files;

	unsigned _radius;

	QString _errorString;
	int _errorLine;
};

#endif // POI_H
