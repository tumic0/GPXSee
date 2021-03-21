#include <QFile>
#include <QDir>
#include "common/rectc.h"
#include "common/greatcircle.h"
#include "data.h"
#include "dem.h"
#include "path.h"
#include "area.h"
#include "common/wgs84.h"
#include "poi.h"


static bool cb(size_t data, void* context)
{
	QSet<int> *set = (QSet<int>*) context;
	set->insert((int)data);

	return true;
}

POI::File::File(int start, int end, const QVector<Waypoint> &data)
  : _enabled(true)
{
	qreal c[2];

	for (int i = start; i <= end; i++) {
		const Coordinates &p = data.at(i).coordinates();

		c[0] = p.lon();
		c[1] = p.lat();
		_tree.Insert(c, c, i);
	}
}

void POI::File::search(const RectC &rect, QSet<int> &set) const
{
	qreal min[2], max[2];

	if (_enabled) {
		if (rect.left() > rect.right()) {
			min[0] = rect.topLeft().lon();
			min[1] = rect.bottomRight().lat();
			max[0] = 180.0;
			max[1] = rect.topLeft().lat();
			_tree.Search(min, max, cb, &set);

			min[0] = -180.0;
			min[1] = rect.bottomRight().lat();
			max[0] = rect.bottomRight().lon();
			max[1] = rect.topLeft().lat();
			_tree.Search(min, max, cb, &set);
		} else {
			min[0] = rect.topLeft().lon();
			min[1] = rect.bottomRight().lat();
			max[0] = rect.bottomRight().lon();
			max[1] = rect.topLeft().lat();
			_tree.Search(min, max, cb, &set);
		}
	}
}


POI::POI(QObject *parent) : QObject(parent)
{
	_errorLine = 0;
	_radius = 1000;
}

POI::~POI()
{
	qDeleteAll(_files);
}

bool POI::loadFile(const QString &path)
{
	Data data(path);

	if (!data.isValid()) {
		_errorString = data.errorString();
		_errorLine = data.errorLine();
		return false;
	}

	int start = _data.size();
	_data.append(data.waypoints());

	_files.insert(path, new File(start, _data.size() - 1, _data));

	emit pointsChanged();

	return true;
}

TreeNode<QString> POI::loadDir(const QString &path)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsFirst);
	QFileInfoList fl = md.entryInfoList();
	TreeNode<QString> tree(md.dirName());

	for (int i = 0; i < fl.size(); i++) {
		const QFileInfo &fi = fl.at(i);

		if (fi.isDir()) {
			TreeNode<QString> child(loadDir(fi.absoluteFilePath()));
			if (!child.isEmpty())
				tree.addChild(child);
		} else {
			if (loadFile(fi.absoluteFilePath()))
				tree.addItem(fi.absoluteFilePath());
			else
				qWarning("%s: %s", qPrintable(fi.absoluteFilePath()),
				  qPrintable(_errorString));
		}
	}

	return tree;
}

void POI::search(const RectC &rect, QSet<int> &set) const
{
	for (ConstIterator it = _files.constBegin(); it != _files.constEnd(); ++it)
		(*it)->search(rect, set);
}

QList<Waypoint> POI::points(const Path &path) const
{
	QList<Waypoint> ret;
	QSet<int> set;
	QSet<int>::const_iterator it;


	for (int i = 0; i < path.count(); i++) {
		const PathSegment &segment = path.at(i);

		for (int j = 1; j < segment.size(); j++) {
			double ds = segment.at(j).distance() - segment.at(j-1).distance();
			unsigned n = (unsigned)ceil(ds / _radius);

			if (n > 1) {
				GreatCircle gc(segment.at(j-1).coordinates(),
				  segment.at(j).coordinates());
				for (unsigned k = 0; k < n; k++) {
					RectC br(gc.pointAt((double)k/n), _radius);
					search(br, set);
				}
			} else {
				RectC br(segment.at(j-1).coordinates(), _radius);
				search(br, set);
			}
		}
	}

	RectC br(path.last().last().coordinates(), _radius);
	search(br, set);


	for (it = set.constBegin(); it != set.constEnd(); ++it)
		ret.append(_data.at(*it));

	return ret;
}

QList<Waypoint> POI::points(const Waypoint &point) const
{
	QList<Waypoint> ret;
	QSet<int> set;
	QSet<int>::const_iterator it;

	RectC br(point.coordinates(), _radius);
	search(br, set);

	for (it = set.constBegin(); it != set.constEnd(); ++it)
		ret.append(_data.at(*it));

	return ret;
}

QList<Waypoint> POI::points(const RectC &rect) const
{
	QList<Waypoint> ret;
	QSet<int> set;
	QSet<int>::const_iterator it;

	double offset = rad2deg(_radius / WGS84_RADIUS);
	RectC br(rect.adjusted(-offset, offset, offset, -offset));
	search(br, set);

	for (it = set.constBegin(); it != set.constEnd(); ++it)
		ret.append(_data.at(*it));

	return ret;
}

bool POI::enableFile(const QString &fileName, bool enable)
{
	Iterator it = _files.find(fileName);
	if (it == _files.end())
		return false;

	(*it)->enable(enable);

	emit pointsChanged();

	return true;
}

void POI::setRadius(unsigned radius)
{
	_radius = radius;

	emit pointsChanged();
}
