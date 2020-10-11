#include <QStringList>
#include "pcs.h"
#include "crs.h"

Projection CRS::projection(const QString &crs)
{
	QStringList list(crs.split(':'));
	QString authority, code;
	bool res;
	int epsg;
	const PCS *pcs;
	const GCS *gcs;

	switch (list.size()) {
		case 2:
			authority = list.at(0);
			code = list.at(1);
			break;
		case 7:
			authority = list.at(4);
			code = list.at(6);
			break;
		case 8:
			authority = list.at(4);
			code = list.at(7);
			break;
		default:
			return Projection();
	}

	if (!authority.compare("EPSG", Qt::CaseInsensitive)) {
		epsg = code.toInt(&res);
		if (!res)
			return Projection();

		if ((pcs = PCS::pcs(epsg)))
			return Projection(pcs);
		else if ((gcs = GCS::gcs(epsg)))
			return Projection(gcs);
		else
			return Projection();
	} else if (!authority.compare("OGC", Qt::CaseInsensitive)) {
		if (code == "CRS84")
			return Projection(GCS::gcs(4326), CoordinateSystem::XY);
		else
			return Projection();
	} else
		return Projection();
}
