#include <QFile>
#include <QSet>
#include <QList>
#include "ll.h"
#include "poi.h"


#define BOUNDING_RECT_SIZE 0.01

bool POI::loadFile(const QString &fileName)
{
	QFile file(fileName);
	bool ret;
	int ln = 1, cnt = _data.size();

	_error.clear();
	_errorLine = 0;

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

		Entry entry;
		entry.description = QString::fromUtf8(ba.data(), ba.size());
		entry.coordinates = ll2mercator(QPointF(lon, lat));

		_data.append(entry);
		ln++;
	}

	for (int i = cnt; i < _data.size(); ++i) {
		qreal c[2];
		c[0] = _data.at(i).coordinates.x();
		c[1] = _data.at(i).coordinates.y();
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

QVector<Entry> POI::points(const QVector<QPointF> &path) const
{
	QVector<Entry> ret;
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
