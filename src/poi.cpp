#include <QFile>
#include <QSet>
#include <QList>
#include "pathitem.h"
#include "waypointitem.h"
#include "ll.h"
#include "gpx.h"
#include "poi.h"


POI::POI(QObject *parent) : QObject(parent)
{
	_errorLine = 0;
	_radius = 0.01;
}

bool POI::loadFile(const QString &fileName)
{
	QString error;
	int errorLine;

	_error.clear();
	_errorLine = 0;

	if (loadCSVFile(fileName)) {
		emit pointsChanged();
		return true;
	} else {
		error = _error;
		errorLine = _errorLine;
	}
	if (loadGPXFile(fileName)) {
		emit pointsChanged();
		return true;
	}

	fprintf(stderr, "Error loading POI file: %s:\n", qPrintable(fileName));
	fprintf(stderr, "CSV: line %d: %s\n", errorLine, qPrintable(error));
	fprintf(stderr, "GPX: line %d: %s\n", _errorLine, qPrintable(_error));

	if (errorLine > _errorLine) {
		_errorLine = errorLine;
		_error = error;
	}

	return false;
}

bool POI::loadGPXFile(const QString &fileName)
{
	GPX gpx;
	FileIndex index;

	index.enabled = true;
	index.start = _data.size();

	if (gpx.loadFile(fileName)) {
		for (int i = 0; i < gpx.waypoints().size(); i++)
			_data.append(gpx.waypoints().at(i));
		index.end = _data.size() - 1;

		for (int i = index.start; i <= index.end; i++) {
			const QPointF &p = _data.at(i).coordinates();
			qreal c[2];
			c[0] = p.x();
			c[1] = p.y();
			_tree.Insert(c, c, i);
		}

		_files.append(fileName);
		_indexes.append(index);

		return true;
	} else {
		_error = gpx.errorString();
		_errorLine = gpx.errorLine();
	}

	return false;
}

bool POI::loadCSVFile(const QString &fileName)
{
	QFile file(fileName);
	FileIndex index;
	bool ret;
	int ln = 1;

	index.enabled = true;
	index.start = _data.size();

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_error = qPrintable(file.errorString());
		return false;
	}

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() < 3) {
			_error = "Parse error.";
			_errorLine = ln;
			return false;
		}

		qreal lat = list[0].trimmed().toDouble(&ret);
		if (!ret) {
			_error = "Invalid latitude.";
			_errorLine = ln;
			return false;
		}
		qreal lon = list[1].trimmed().toDouble(&ret);
		if (!ret) {
			_error = "Invalid longitude.";
			_errorLine = ln;
			return false;
		}
		Waypoint wp(QPointF(lon, lat));

		QByteArray ba = list[2].trimmed();
		QString name = QString::fromUtf8(ba.data(), ba.size());
		wp.setName(name);

		if (list.size() > 3) {
			ba = list[3].trimmed();
			wp.setDescription(QString::fromUtf8(ba.data(), ba.size()));
		}

		_data.append(wp);
		ln++;
	}
	index.end = _data.size() - 1;

	for (int i = index.start; i <= index.end; i++) {
		const QPointF &p = _data.at(i).coordinates();
		qreal c[2];
		c[0] = p.x();
		c[1] = p.y();
		_tree.Insert(c, c, i);
	}

	_files.append(fileName);
	_indexes.append(index);

	return true;
}

static bool cb(size_t data, void* context)
{
	QSet<int> *set = (QSet<int>*) context;
	set->insert((int)data);

	return true;
}

QVector<Waypoint> POI::points(const PathItem *path) const
{
	QVector<Waypoint> ret;
	QSet<int> set;
	qreal min[2], max[2];
	const QPainterPath &pp = path->path();

	for (int i = 0; i < pp.elementCount(); i++) {
		QPointF p = mercator2ll(pp.elementAt(i));
		min[0] = p.x() - _radius;
		min[1] = -p.y() - _radius;
		max[0] = p.x() + _radius;
		max[1] = -p.y() + _radius;
		_tree.Search(min, max, cb, &set);
	}

	QSet<int>::const_iterator i = set.constBegin();
	while (i != set.constEnd()) {
		ret.append(_data.at(*i));
		++i;
	}

	return ret;
}

QVector<Waypoint> POI::points(const QList<WaypointItem*> &list)
  const
{
	QVector<Waypoint> ret;
	QSet<int> set;
	qreal min[2], max[2];

	for (int i = 0; i < list.count(); i++) {
		const QPointF &p = list.at(i)->waypoint().coordinates();
		min[0] = p.x() - _radius;
		min[1] = p.y() - _radius;
		max[0] = p.x() + _radius;
		max[1] = p.y() + _radius;
		_tree.Search(min, max, cb, &set);
	}

	QSet<int>::const_iterator i = set.constBegin();
	while (i != set.constEnd()) {
		ret.append(_data.at(*i));
		++i;
	}

	return ret;
}

QVector<Waypoint> POI::points(const QList<Waypoint> &list) const
{
	QVector<Waypoint> ret;
	QSet<int> set;
	qreal min[2], max[2];

	for (int i = 0; i < list.count(); i++) {
		const QPointF &p = list.at(i).coordinates();
		min[0] = p.x() - _radius;
		min[1] = p.y() - _radius;
		max[0] = p.x() + _radius;
		max[1] = p.y() + _radius;
		_tree.Search(min, max, cb, &set);
	}

	QSet<int>::const_iterator i = set.constBegin();
	while (i != set.constEnd()) {
		ret.append(_data.at(*i));
		++i;
	}

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
			const QPointF &p = _data.at(j).coordinates();
			qreal c[2];
			c[0] = p.x();
			c[1] = p.y();
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

void POI::setRadius(qreal radius)
{
	_radius = radius;

	emit pointsChanged();
}
