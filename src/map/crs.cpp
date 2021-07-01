#include <QStringList>
#include "pcs.h"
#include "crs.h"

Projection CRS::projection(const QString &crs)
{
	QStringList list(crs.split(':'));
	QString authority, code;
	bool res;

	switch (list.size()) {
		case 2:
			authority = list.at(0);
			code = list.at(1);
			break;
		case 6:
			authority = list.at(4);
			code = list.at(5);
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
		int epsg = code.toInt(&res);
		if (!res)
			return Projection();

		PCS pcs(PCS::pcs(epsg));
		if (!pcs.isNull())
			return Projection(pcs);

		GCS gcs(GCS::gcs(epsg));
		if (!gcs.isNull())
			return Projection(gcs);

		return Projection();
	} else if (!authority.compare("OGC", Qt::CaseInsensitive)) {
		if (code == "CRS84")
			return Projection(GCS::gcs(4326), CoordinateSystem::XY);
		else
			return Projection();
	} else
		return Projection();
}

Projection CRS::projection(int id)
{
	PCS pcs(PCS::pcs(id));
	if (!pcs.isNull())
		return Projection(pcs);

	GCS gcs(GCS::gcs(id));
	if (!gcs.isNull())
		return Projection(gcs);

	return Projection();
}
