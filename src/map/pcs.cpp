#include <QFile>
#include "angularunits.h"
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

static bool parameter(int key, double val, int units, Projection::Setup &setup)
{
	switch (key) {
		case 8801:
		case 8821:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setLatitudeOrigin(au.toDegrees(val));}
			return true;
		case 8802:
		case 8822:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setLongitudeOrigin(au.toDegrees(val));}
			return true;
		case 8805:
			setup.setScale(val);
			return true;
		case 8806:
		case 8826:
			{LinearUnits lu(units);
			if (lu.isNull())
				return false;
			setup.setFalseEasting(lu.toMeters(val));}
			return true;
		case 8807:
		case 8827:
			{LinearUnits lu(units);
			if (lu.isNull())
				return false;
			setup.setFalseNorthing(lu.toMeters(val));}
			return true;
		case 8823:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setStandardParallel1(au.toDegrees(val));}
			return true;
		case 8824:
			{AngularUnits au(units);
			if (au.isNull())
				return false;
			setup.setStandardParallel2(au.toDegrees(val));}
			return true;
		default:
			return true;
	}
}

static int projectionSetup(const QList<QByteArray> &list,
  Projection::Setup &setup)
{
	bool res;

	for (int i = 6; i < 27; i += 3) {
		int key = list[i].trimmed().toInt(&res);
		double val = list[i+1].trimmed().toDouble(&res);
		int un = list[i+2].trimmed().toInt(&res);

		if (!parameter(key, val, un, setup))
			return (i - 6)/3;
	}

	return 0;
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
	int ln = 0, pn;
	const GCS *gcs;

	if (!file.open(QFile::ReadOnly)) {
		qWarning("Error opening PCS file: %s: %s", qPrintable(path),
		  qPrintable(file.errorString()));
		return;
	}

	while (!file.atEnd()) {
		ln++;

		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() != 27) {
			qWarning("%s: %d: Format error", qPrintable(path), ln);
			continue;
		}

		int id = list[1].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid PCS code", qPrintable(path), ln);
			continue;
		}
		int gcsid = list[2].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid GCS code", qPrintable(path), ln);
			continue;
		}
		int proj = list[3].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid projection code", qPrintable(path), ln);
			continue;
		}
		int units = list[4].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid linear units code", qPrintable(path), ln);
			continue;
		}
		int transform = list[5].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid coordinate transformation code",
			  qPrintable(path), ln);
			continue;
		}

		Projection::Setup setup;
		if ((pn = projectionSetup(list, setup))) {
			qWarning("%s: %d: Invalid projection parameter #%d",
			  qPrintable(path), ln, pn);
			continue;
		}

		if (!(gcs = GCS::gcs(gcsid))) {
			qWarning("%s: %d: Unknown GCS code", qPrintable(path), ln);
			continue;
		}

		PCS pcs(gcs, transform, setup, units);
		if (!pcs.isValid())
			qWarning("%s: %d: Unknown coordinate transformation/linear units code",
			  qPrintable(path), ln);
		else
			_pcss.append(Entry(id, proj, pcs));
	}
}

QDebug operator<<(QDebug dbg, const PCS &pcs)
{
	dbg.nospace() << "PCS(" << *pcs.gcs() << ", " << pcs.method() << ", "
	  << pcs.units() << ", " << pcs.setup() << ")";
	return dbg.maybeSpace();
}
