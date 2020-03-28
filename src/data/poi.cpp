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


POI::POI(QObject *parent) : QObject(parent)
{
	_errorLine = 0;
	_radius = 1000;
}

bool POI::loadFile(const QString &path)
{
	Data data(path);
	FileIndex index;

	index.enabled = true;
	index.start = _data.size();

	if (!data.isValid()) {
		_errorString = data.errorString();
		_errorLine = data.errorLine();
		return false;
	}

	for (int i = 0; i < data.waypoints().size(); i++)
		_data.append(data.waypoints().at(i));
	index.end = _data.size() - 1;

	for (int i = index.start; i <= index.end; i++) {
		const Coordinates &p = _data.at(i).coordinates();
		qreal c[2];
		c[0] = p.lon();
		c[1] = p.lat();
		_tree.Insert(c, c, i);
	}

	_files.append(path);
	_indexes.append(index);

	emit pointsChanged();

	return true;
}

void POI::loadDir(const QString &path)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList fl = md.entryInfoList();

	for (int i = 0; i < fl.size(); i++) {
		const QFileInfo &fi = fl.at(i);

		if (fi.isDir())
			loadDir(fi.absoluteFilePath());
		else
			if (!loadFile(fi.absoluteFilePath()))
				qWarning("%s: %s", qPrintable(fi.absoluteFilePath()),
				  qPrintable(_errorString));
	}
}

static bool cb(size_t data, void* context)
{
	QSet<int> *set = (QSet<int>*) context;
	set->insert((int)data);

	return true;
}

void POI::search(const RectC &rect, QSet<int> &set) const
{
	qreal min[2], max[2];

	min[0] = rect.topLeft().lon();
	min[1] = rect.bottomRight().lat();
	max[0] = rect.bottomRight().lon();
	max[1] = rect.topLeft().lat();

	_tree.Search(min, max, cb, &set);
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
	qreal min[2], max[2];
	QSet<int>::const_iterator it;

	RectC br(point.coordinates(), _radius);
	min[0] = br.topLeft().lon();
	min[1] = br.bottomRight().lat();
	max[0] = br.bottomRight().lon();
	max[1] = br.topLeft().lat();

	_tree.Search(min, max, cb, &set);

	for (it = set.constBegin(); it != set.constEnd(); ++it)
		ret.append(_data.at(*it));

	return ret;
}

QList<Waypoint> POI::points(const Area &area) const
{
	QList<Waypoint> ret;
	qreal min[2], max[2];
	QSet<int> set;
	QSet<int>::const_iterator it;

	RectC br(area.boundingRect());
	double offset = rad2deg(_radius / WGS84_RADIUS);

	min[0] = br.topLeft().lon() - offset;
	min[1] = br.bottomRight().lat() - offset;
	max[0] = br.bottomRight().lon() + offset;
	max[1] = br.topLeft().lat() + offset;

	_tree.Search(min, max, cb, &set);

	for (it = set.constBegin(); it != set.constEnd(); ++it)
		ret.append(_data.at(*it));

	return ret;
}

void POI::enableFile(const QString &fileName, bool enable)
{
	int i;

	i = _files.indexOf(fileName);
	Q_ASSERT(i >= 0);
	_indexes[i].enabled = enable;

	_tree.RemoveAll();
	for (int i = 0; i < _indexes.count(); i++) {
		FileIndex idx = _indexes.at(i);
		if (!idx.enabled)
			continue;

		for (int j = idx.start; j <= idx.end; j++) {
			const Coordinates &p = _data.at(j).coordinates();
			qreal c[2];
			c[0] = p.lon();
			c[1] = p.lat();
			_tree.Insert(c, c, j);
		}
	}

	emit pointsChanged();
}

void POI::clear()
{
	_tree.RemoveAll();
	_data.clear();
	_files.clear();
	_indexes.clear();

	emit pointsChanged();
}

void POI::setRadius(unsigned radius)
{
	_radius = radius;

	emit pointsChanged();
}
