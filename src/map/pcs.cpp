#include <QFile>
#include "pcs.h"


class PCS::Entry {
public:
	Entry(int id, int gcs, int proj, const PCS &pcs)
	  : _id(id), _gcs(gcs), _proj(proj), _pcs(pcs) {}

	int id() const {return _id;}
	int gcs() const {return _gcs;}
	int proj() const {return _proj;}
	const PCS &pcs() const {return _pcs;}

private:
	int _id;
	int _gcs;
	int _proj;
	PCS _pcs;
};

QList<PCS::Entry> PCS::_pcss;
QString PCS::_errorString;
int PCS::_errorLine;

static double parameter(const QString &str, bool *res)
{
	QString field = str.trimmed();
	if (field.isEmpty()) {
		*res = true;
		return NAN;
	}

	return field.toDouble(res);
}

PCS::PCS(int id)
{
	for (int i = 0; i < _pcss.size(); i++) {
		if (_pcss.at(i).id() == id) {
			*this = _pcss.at(i).pcs();
			return;
		}
	}

	*this = PCS();
}

PCS::PCS(int gcs, int proj)
{
	for (int i = 0; i < _pcss.size(); i++) {
		if (_pcss.at(i).gcs() == gcs && _pcss.at(i).proj() == proj) {
			*this = _pcss.at(i).pcs();
			return;
		}
	}

	*this = PCS();
}

void PCS::error(const QString &str)
{
	_errorString = str;
	_pcss.clear();
}

bool PCS::loadList(const QString &path)
{
	QFile file(path);
	bool res[12];
	int id, gcs, proj, transform;


	if (!file.open(QFile::ReadOnly)) {
		error(file.errorString());
		return false;
	}

	_errorLine = 1;
	_errorString.clear();

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() != 12) {
			error("Format error");
			return false;
		}

		id = list[1].trimmed().toInt(&res[1]);;
		gcs = list[2].trimmed().toInt(&res[2]);
		proj = list[3].trimmed().toInt(&res[3]);
		transform = list[4].trimmed().toInt(&res[4]);

		Projection::Setup setup(
		  parameter(list[5], &res[5]), parameter(list[6], &res[6]),
		  parameter(list[7], &res[7]), parameter(list[8], &res[8]),
		  parameter(list[9], &res[9]), parameter(list[10], &res[10]),
		  parameter(list[11], &res[11]));

		for (int i = 1; i < 12; i++) {
			if (!res[i]) {
				error("Parse error");
				return false;
			}
		}

		Datum datum(gcs);
		if (datum.isNull()) {
			error("Unknown datum");
			return false;
		}
		Projection::Method method(transform);
		if (method.isNull()) {
			error("Unknown coordinates transformation method");
			return false;
		}

		_pcss.append(Entry(id, gcs, proj, PCS(datum, method, setup)));

		_errorLine++;
	}

	return true;
}

QDebug operator<<(QDebug dbg, const PCS &pcs)
{
	dbg.nospace() << "PCS(" << pcs.datum() << ", " << pcs.method()
	  << ", " << pcs.setup() << ")";
	return dbg.space();
}
