#include <QFile>
#include <QSet>
#include <QList>
#include "ll.h"
#include "gpx.h"
#include "poi.h"


#define BOUNDING_RECT_SIZE 0.01

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
	int cnt = _data.size();

	if (gpx.loadFile(fileName)) {
		for (int i = 0; i < gpx.waypoints().size(); i++)
			_data.append(WayPoint(
			  ll2mercator(gpx.waypoints().at(i).coordinates()),
			  gpx.waypoints().at(i).description()));

		for (int i = cnt; i < _data.size(); ++i) {
			qreal c[2];
			c[0] = _data.at(i).coordinates().x();
			c[1] = _data.at(i).coordinates().y();
			_tree.Insert(c, c, i);
		}

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
	bool ret;
	int ln = 1, cnt = _data.size();

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

		_data.append(WayPoint(ll2mercator(QPointF(lon, lat)),
		  QString::fromUtf8(ba.data(), ba.size())));
		ln++;
	}

	for (int i = cnt; i < _data.size(); ++i) {
		qreal c[2];
		c[0] = _data.at(i).coordinates().x();
		c[1] = _data.at(i).coordinates().y();
		_tree.Insert(c, c, i);
	}

	return true;
}

static bool cb(size_t data, void* context)
{
	QSet<int> *set = (QSet<int>*) context;
	set->insert((int)data);

	return true;
}

QVector<WayPoint> POI::points(const QVector<QPointF> &path) const
{
	QVector<WayPoint> ret;
	QSet<int> set;
	qreal min[2], max[2];

	for (int i = 0; i < path.count(); i++) {
		min[0] = path.at(i).x() - BOUNDING_RECT_SIZE;
		min[1] = path.at(i).y() - BOUNDING_RECT_SIZE;
		max[0] = path.at(i).x() + BOUNDING_RECT_SIZE;
		max[1] = path.at(i).y() + BOUNDING_RECT_SIZE;
		_tree.Search(min, max, cb, &set);
	}

	QSet<int>::const_iterator i = set.constBegin();
	while (i != set.constEnd()) {
		ret.append(_data.at(*i));
		++i;
	}

	return ret;
}

void POI::clear()
{
	_tree.RemoveAll();
	_data.clear();
}
