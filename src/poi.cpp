#include <QFile>
#include <QSet>
#include <QList>
#include "ll.h"
#include "gpx.h"
#include "poi.h"


bool POI::loadFile(const QString &fileName)
{
	QString error;
	int errorLine;

	_error.clear();
	_errorLine = 0;


	if (loadCSVFile(fileName))
		return true;
	else {
		error = _error;
		errorLine = _errorLine;
	}
	if (loadGPXFile(fileName))
		return true;

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
			_data.append(Waypoint(gpx.waypoints().at(i).coordinates(),
			  gpx.waypoints().at(i).description()));
		index.end = _data.size() - 1;

		for (int i = index.start; i <= index.end; i++) {
			qreal c[2];
			c[0] = _data.at(i).coordinates().x();
			c[1] = _data.at(i).coordinates().y();
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
			_error = "Parse error";
			_errorLine = ln;
			return false;
		}

		qreal lat = list[0].trimmed().toDouble(&ret);
		if (!ret) {
			_error = "Invalid latitude";
			_errorLine = ln;
			return false;
		}
		qreal lon = list[1].trimmed().toDouble(&ret);
		if (!ret) {
			_error = "Invalid longitude";
			_errorLine = ln;
			return false;
		}
		QByteArray ba = list[2].trimmed();

		_data.append(Waypoint(QPointF(lon, lat),
		  QString::fromUtf8(ba.data(), ba.size())));
		ln++;
	}
	index.end = _data.size() - 1;

	for (int i = index.start; i <= index.end; i++) {
		qreal c[2];
		c[0] = _data.at(i).coordinates().x();
		c[1] = _data.at(i).coordinates().y();
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

QVector<Waypoint> POI::points(const QVector<QPointF> &path, qreal radius) const
{
	QVector<Waypoint> ret;
	QSet<int> set;
	qreal min[2], max[2];

	for (int i = 0; i < path.count(); i++) {
		const QPointF &p = path.at(i);
		min[0] = p.x() - radius;
		min[1] = p.y() - radius;
		max[0] = p.x() + radius;
		max[1] = p.y() + radius;
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
			qreal c[2];
			c[0] = _data.at(j).coordinates().x();
			c[1] = _data.at(j).coordinates().y();
			_tree.Insert(c, c, j);
		}
	}
}

void POI::clear()
{
	_tree.RemoveAll();
	_data.clear();
	_files.clear();
	_indexes.clear();
}
