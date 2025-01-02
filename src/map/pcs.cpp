#include <QFile>
#include "common/csv.h"
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
	QMap<int, Entry>::const_iterator it(_pcss.find(id));

	if (it == _pcss.constEnd())
		return PCS();
	else {
		const Entry &e = it.value();
		return PCS(GCS::gcs(e.gcs()), Conversion::conversion(e.conversion()));
	}
}

bool PCS::loadList(const QString &path)
{
	QFile file(path);
	CSV csv(&file);
	QByteArrayList entry;
	bool res;

	if (!file.open(QFile::ReadOnly)) {
		qWarning("%s: %s", qPrintable(path), qPrintable(file.errorString()));
		return false;
	}

	while (!csv.atEnd()) {
		if (!csv.readEntry(entry)) {
			qWarning("%s:%d: Parse error", qPrintable(path), csv.line());
			return false;
		}
		if (entry.size() < 4) {
			qWarning("%s:%d: Invalid column count", qPrintable(path),
			  csv.line() - 1);
			return false;
		}

		QString name(entry.at(0));
		int id = entry.at(1).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid PCS code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int gcs = entry.at(2).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid GCS code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		int proj = entry.at(3).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid projection code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}

		if (GCS::gcs(gcs).isNull()) {
			qWarning("%s:%d: Unknown GCS code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		if (Conversion::conversion(proj).isNull()) {
			qWarning("%s:%d: Unknown projection code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}

		_pcss.insert(id, Entry(name, gcs, proj));
	}

	return true;
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
