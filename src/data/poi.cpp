#include <QFile>
#include <QDir>
#include "common/rectc.h"
#include "data.h"
#include "poi.h"


POI::POI(QObject *parent) : QObject(parent)
{
	_errorLine = 0;
	_radius = 1000;
}

bool POI::loadFile(const QString &path, bool dir)
{
	Data data;
	FileIndex index;

	index.enabled = true;
	index.start = _data.size();

	if (!data.loadFile(path)) {
		if (dir) {
			if (data.errorLine())
				_errorString += QString("%1:%2: %3\n").arg(path)
				  .arg(data.errorLine()).arg(data.errorString());
			else
				_errorString += path + ": " + data.errorString() + "\n";
		} else {
			_errorString = data.errorString();
			_errorLine = data.errorLine();
		}
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

bool POI::loadFile(const QString &path)
{
	_errorString.clear();
	_errorLine = 0;

	return loadFile(path, false);
}

bool POI::loadDir(const QString &path)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList fl = md.entryInfoList();
	bool ret = true;

	_errorString.clear();
	_errorLine = 0;

	for (int i = 0; i < fl.size(); i++) {
		const QFileInfo &fi = fl.at(i);

		if (fi.isDir()) {
			if (!loadDir(fi.absoluteFilePath()))
				ret = false;
		} else {
			if (!loadFile(fi.absoluteFilePath(), true))
				ret = false;
		}
	}

	return ret;
}

static bool cb(size_t data, void* context)
{
	QSet<int> *set = (QSet<int>*) context;
	set->insert((int)data);

	return true;
}

static Coordinates intermediate(const Coordinates &c1, const Coordinates &c2,
  double f)
{
	double lat1 = deg2rad(c1.lat());
	double lon1 = deg2rad(c1.lon());
	double lat2 = deg2rad(c2.lat());
	double lon2 = deg2rad(c2.lon());

	double cosLat1 = cos(lat1);
	double cosLat2 = cos(lat2);
	double d = 2 * asin(sqrt(pow((sin((lat1 - lat2) / 2)), 2) + cosLat1
	  * cosLat2 * pow(sin((lon1 - lon2) / 2), 2)));
	double sinD = sin(d);
	double A = sin((1.0-f)*d) / sinD;
	double B = sin(f*d) / sinD;
	double x = A * cosLat1 * cos(lon1) + B * cosLat2 * cos(lon2);
	double y = A * cosLat1 * sin(lon1) + B * cosLat2 * sin(lon2);
	double z = A * sin(lat1) + B * sin(lat2);

	return Coordinates(rad2deg(atan2(y, x)), rad2deg(atan2(z, sqrt(x*x + y*y))));
}

QList<Waypoint> POI::points(const Path &path) const
{
	QList<Waypoint> ret;
	QSet<int> set;
	qreal min[2], max[2];
	QSet<int>::const_iterator it;


	for (int i = 1; i < path.count(); i++) {
		double ds = path.at(i).distance() - path.at(i-1).distance();
		unsigned n = (unsigned)ceil(ds / _radius);
		for (unsigned j = 0; j < n; j++) {
			RectC br(n > 1 ? intermediate(path.at(i-1).coordinates(),
			  path.at(i).coordinates(), (double)j/(double)n)
			  : path.at(i-1).coordinates(), _radius);

			min[0] = br.topLeft().lon();
			min[1] = br.bottomRight().lat();
			max[0] = br.bottomRight().lon();
			max[1] = br.topLeft().lat();

			_tree.Search(min, max, cb, &set);
		}
	}

	RectC br(path.last().coordinates(), _radius);
	min[0] = br.topLeft().lon();
	min[1] = br.bottomRight().lat();
	max[0] = br.bottomRight().lon();
	max[1] = br.topLeft().lat();

	_tree.Search(min, max, cb, &set);


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
