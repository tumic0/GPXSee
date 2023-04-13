#include <QFile>
#include "angularunits.h"
#include "pcs.h"

QMap<int, PCS::Entry> PCS::_pcss = defaults();

QMap<int, PCS::Entry> PCS::defaults()
{
	QMap<int, Entry> map;
	map.insert(3857, Entry("WGS 84 / Pseudo-Mercator", 4326, 3856));
	return map;
}

PCS PCS::pcs(int id)
{
	QMap<int, Entry>::const_iterator it = _pcss.find(id);

	if (it == _pcss.constEnd())
		return PCS();
	else {
		const Entry &e = it.value();
		return PCS(GCS::gcs(e.gcs()), Conversion::conversion(e.proj()));
	}
}

void PCS::loadList(const QString &path)
{
	QFile file(path);
	bool res;
	int ln = 0;

	if (!file.open(QFile::ReadOnly)) {
		qWarning("Error opening PCS file: %s: %s", qPrintable(path),
		  qPrintable(file.errorString()));
		return;
	}

	while (!file.atEnd()) {
		ln++;

		QByteArray line = file.readLine(4096);
		QList<QByteArray> list = line.split(',');
		if (list.size() != 4) {
			qWarning("%s:%d: Format error", qPrintable(path), ln);
			continue;
		}

		QString name(list.at(0).trimmed());
		int id = list.at(1).trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid PCS code", qPrintable(path), ln);
			continue;
		}
		int gcs = list.at(2).trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid GCS code", qPrintable(path), ln);
			continue;
		}
		int proj = list.at(3).trimmed().toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid projection code", qPrintable(path), ln);
			continue;
		}

		if (GCS::gcs(gcs).isNull()) {
			qWarning("%s:%d: Unknown GCS code", qPrintable(path), ln);
			continue;
		}

		_pcss.insert(id, Entry(name, gcs, proj));
	}
}

QList<KV<int, QString> > PCS::list()
{
	QList<KV<int, QString> > list;

	for (QMap<int, Entry>::const_iterator it = _pcss.constBegin();
	  it != _pcss.constEnd(); ++it)
		list.append(KV<int, QString>(it.key(), it.value().name()));

	return list;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PCS &pcs)
{
	dbg.nospace() << "PCS(" << pcs.gcs() << ", " << pcs.conversion() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
