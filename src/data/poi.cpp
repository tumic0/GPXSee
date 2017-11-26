#include <QFile>
#include "data.h"
#include "poi.h"


POI::POI(QObject *parent) : QObject(parent)
{
	_errorLine = 0;
	_radius = 1000;
}

bool POI::loadFile(const QString &fileName)
{
	Data data;
	FileIndex index;

	_errorString.clear();
	_errorLine = 0;

	index.enabled = true;
	index.start = _data.size();

	if (!data.loadFile(fileName)) {
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

	_files.append(fileName);
	_indexes.append(index);

	emit pointsChanged();

	return true;
}

static bool cb(size_t data, void* context)
{
	QSet<int> *set = (QSet<int>*) context;
	set->insert((int)data);

	return true;
}

QList<Waypoint> POI::points(const Path &path) const
{
	QList<Waypoint> ret;
	QSet<int> set;
	qreal min[2], max[2];
	QSet<int>::const_iterator it;

	for (int i = 0; i < path.count(); i++) {
		const Coordinates &c = path.at(i).coordinates();
		QPair<Coordinates, Coordinates> br = c.boundingRect(_radius);
		min[0] = br.first.lon();
		min[1] = br.first.lat();
		max[0] = br.second.lon();
		max[1] = br.second.lat();

		_tree.Search(min, max, cb, &set);
	}

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

	const Coordinates &c = point.coordinates();

	QPair<Coordinates, Coordinates> br = c.boundingRect(_radius);
	min[0] = br.first.lon();
	min[1] = br.first.lat();
	max[0] = br.second.lon();
	max[1] = br.second.lat();

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
