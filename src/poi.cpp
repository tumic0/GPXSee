#include <QFile>
#include <QSet>
#include <QList>
#include "ll.h"
#include "poi.h"


bool POI::loadFile(const QString &fileName)
{
	QFile file(fileName);
	bool ret;
	int ln = 1;

	_error.clear();

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_error = qPrintable(file.errorString());
		return false;
	}

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() < 3) {
			_error = QString("Parse error\nLine %1").arg(ln);
			return false;
		}

		qreal lat = list[0].trimmed().toDouble(&ret);
		if (!ret) {
			_error = QObject::tr("Invalid latitude\nLine %1").arg(ln);
			return false;
		}
		qreal lon = list[1].trimmed().toDouble(&ret);
		if (!ret) {
			_error = QObject::tr("Invalid longitude\nLine %1").arg(ln);
			return false;
		}
		QByteArray ba = list[2].trimmed();

		Entry entry;
		entry.description = QString::fromUtf8(ba.data(), ba.size());
		entry.coordinates = ll2mercator(QPointF(lon, lat));

		_data.append(entry);
		ln++;
	}

	for (int i = 0; i < _data.size(); ++i) {
		qreal c[2];
		c[0] = _data.at(i).coordinates.x();
		c[1] = _data.at(i).coordinates.y();
		_tree.Insert(c, c, &_data.at(i));
	}

	return true;
}

static bool cb(const Entry* data, void* context)
{
	QSet<const Entry*> *set = (QSet<const Entry*>*) context;
	set->insert(data);

	return true;
}

#define RECT 0.01
QVector<Entry> POI::points(const QVector<QPointF> &path) const
{
	QVector<Entry> ret;
	QSet<const Entry*> set;
	qreal min[2], max[2];

	for (int i = 0; i < path.count(); i++) {
		min[0] = path.at(i).x() - RECT;
		min[1] = path.at(i).y() - RECT;
		max[0] = path.at(i).x() + RECT;
		max[1] = path.at(i).y() + RECT;
		_tree.Search(min, max, cb, &set);
	}

	QSet<const Entry *>::const_iterator i = set.constBegin();
	while (i != set.constEnd()) {
		ret.append(*(*i));
    	++i;
	}

	return ret;
}

void POI::clear()
{
	_tree.RemoveAll();
	_data.clear();
}
