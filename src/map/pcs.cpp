#include <QFile>
#include "pcs.h"


class PCS::Entry {
public:
	Entry(int id, int proj, const PCS &pcs) : _id(id), _proj(proj), _pcs(pcs) {}

	int id() const {return _id;}
	int proj() const {return _proj;}
	const PCS &pcs() const {return _pcs;}

private:
	int _id, _proj;
	PCS _pcs;
};

QList<PCS::Entry> PCS::_pcss;

static double parameter(const QString &str, bool *res)
{
	QString field = str.trimmed();
	if (field.isEmpty()) {
		*res = true;
		return NAN;
	}

	return field.toDouble(res);
}


const PCS *PCS::pcs(int id)
{
	for (int i = 0; i < _pcss.size(); i++)
		if (_pcss.at(i).id() == id)
			return &(_pcss.at(i).pcs());

	return 0;
}

const PCS *PCS::pcs(const GCS *gcs, int proj)
{
	for (int i = 0; i < _pcss.size(); i++)
		if (_pcss.at(i).proj() == proj && *(_pcss.at(i).pcs().gcs()) == *gcs)
			return &(_pcss.at(i).pcs());

	return 0;
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

		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() != 12) {
			qWarning("%s: %d: Format error", qPrintable(path), ln);
			continue;
		}

		int id = list[1].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid PCS code", qPrintable(path), ln);
			continue;
		}
		int gcs = list[2].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid GCS code", qPrintable(path), ln);
			continue;
		}
		int proj = list[3].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid projection code", qPrintable(path), ln);
			continue;
		}
		int transform = list[4].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid coordinate transformation code",
			  qPrintable(path), ln);
			continue;
		}
		double lat0 = parameter(list[5], &res);
		if (!res) {
			qWarning("%s: %d: Invalid latitude origin", qPrintable(path), ln);
			continue;
		}
		double lon0 = parameter(list[6], &res);
		if (!res) {
			qWarning("%s: %d: Invalid longitude origin", qPrintable(path), ln);
			continue;
		}
		double scale = parameter(list[7], &res);
		if (!res) {
			qWarning("%s: %d: Invalid scale", qPrintable(path), ln);
			continue;
		}
		double fe = parameter(list[8], &res);
		if (!res) {
			qWarning("%s: %d: Invalid false easting", qPrintable(path), ln);
			continue;
		}
		double fn = parameter(list[9], &res);
		if (!res) {
			qWarning("%s: %d: Invalid false northing", qPrintable(path), ln);
			continue;
		}
		double sp1 = parameter(list[10], &res);
		if (!res) {
			qWarning("%s: %d: Invalid standard parallel #1", qPrintable(path),
			  ln);
			continue;
		}
		double sp2 = parameter(list[11], &res);
		if (!res) {
			qWarning("%s: %d: Invalid standard parallel #2", qPrintable(path),
			  ln);
			continue;
		}

		_pcss.append(Entry(id, proj, PCS(GCS::gcs(gcs), transform,
		  Projection::Setup(lat0, lon0, scale, fe, fn, sp1, sp2))));
	}
}

QDebug operator<<(QDebug dbg, const PCS &pcs)
{
	dbg.nospace() << "PCS(" << *pcs.gcs() << ", " << pcs.method()
	  << ", " << pcs.setup() << ")";
	return dbg.maybeSpace();
}
