#include <QIODevice>
#include "utm.h"
#include "gcs.h"
#include "pcs.h"
#include "mapfile.h"


static double parameter(const QString &str, bool *res, double dflt = 0.0)
{
	QString field = str.trimmed();
	if (field.isEmpty()) {
		*res = true;
		return dflt;
	}

	return field.toDouble(res);
}

int MapFile::parse(QIODevice &device, QList<CalibrationPoint> &points,
  QString &projection, Projection::Setup &setup, QString &datum)
{
	bool res, utm = false;
	int ln = 1, zone = 0;


	while (!device.atEnd()) {
		QByteArray line = device.readLine();

		if (ln == 1) {
			QString fileType(QString::fromUtf8(line).trimmed());
			if (!fileType.startsWith("OziExplorer Map Data File"))
				return ln;
		} else if (ln == 2)
			_name = line.trimmed();
		else if (ln == 3)
			_image = line.trimmed();
		else if (ln == 5)
			datum = line.split(',').at(0).trimmed();
		else {
			QList<QByteArray> list = line.split(',');
			QString key(list.at(0).trimmed());

			if (key.startsWith("Point") && list.count() == 17
			  && !list.at(2).trimmed().isEmpty()) {
				PointD xy;
				xy.rx() = list.at(2).trimmed().toInt(&res);
				if (!res)
					return ln;
				xy.ry() = list.at(3).trimmed().toInt(&res);
				if (!res)
					return ln;

				Coordinates c;
				bool ll = true;
				int latd = list.at(6).trimmed().toInt(&res);
				if (!res)
					ll = false;
				double latm = list.at(7).trimmed().toDouble(&res);
				if (!res)
					ll = false;
				int lond = list.at(9).trimmed().toInt(&res);
				if (!res)
					ll = false;
				double lonm = list.at(10).trimmed().toDouble(&res);
				if (!res)
					ll = false;
				if (ll && list.at(8).trimmed() == "S") {
					latd = -latd;
					latm = -latm;
				}
				if (ll && list.at(11).trimmed() == "W") {
					lond = -lond;
					lonm = -lonm;
				}
				if (ll)
					c = Coordinates(lond + lonm/60.0, latd + latm/60.0);

				PointD pp;
				double ppx = list.at(14).trimmed().toDouble(&res);
				if (res)
					pp.rx() = ppx;
				double ppy = list.at(15).trimmed().toDouble(&res);
				if (res)
					pp.ry() = ppy;

				if (c.isValid())
					points.append(CalibrationPoint(xy, c));
				else if (pp.isValid())
					points.append(CalibrationPoint(xy, pp));
				else
					return ln;

				if (utm && !zone) {
					zone = list.at(13).trimmed().toInt(&res);
					if (res) {
						if (list.at(16).trimmed() == "S")
							zone = -zone;
					} else {
						if (c.isValid())
							zone = UTM::zone(c);
					}
				}
			} else if (key == "IWH") {
				if (list.count() < 4)
					return ln;
				int w = list.at(2).trimmed().toInt(&res);
				if (!res)
					return ln;
				int h = list.at(3).trimmed().toInt(&res);
				if (!res)
					return ln;
				_size = QSize(w, h);
			} else if (key == "Map Projection") {
				if (list.count() < 2)
					return ln;
				projection = list.at(1);
				utm = (projection == "(UTM) Universal Transverse Mercator");
			} else if (key == "Projection Setup") {
				if (list.count() < 8)
					return ln;

				if (utm && zone)
					setup = UTM::setup(zone);
				else {
					bool r[8];
					setup = Projection::Setup(
					  parameter(list[1], &r[1]), parameter(list[2], &r[2]),
					  parameter(list[3], &r[3], 1.0), parameter(list[4], &r[4]),
					  parameter(list[5], &r[5]), parameter(list[6], &r[6]),
					  parameter(list[7], &r[7]));

					for (int i = 1; i < 8; i++)
						if (!r[i])
							return ln;
				}
			}
		}

		ln++;
	}

	return (ln < 9) ? ln : 0;
}

bool MapFile::parseMapFile(QIODevice &device, QList<CalibrationPoint> &points,
  QString &projection, Projection::Setup &setup, QString &datum)
{
	int el;

	if (!device.open(QIODevice::ReadOnly)) {
		_errorString = QString("Error opening file: %1")
		  .arg(device.errorString());
		return false;
	}

	if ((el = parse(device, points, projection, setup, datum)))
		_errorString = QString("Parse error on line %1").arg(el);

	device.close();

	return (!el);
}

bool MapFile::createProjection(const QString &datum, const QString &name,
  const Projection::Setup &setup)
{
	PCS pcs;

	GCS gcs(GCS::gcs(datum));
	if (gcs.isNull()) {
		_errorString = QString("%1: Unknown datum").arg(datum);
		return false;
	}

	if (name == "Latitude/Longitude") {
		_projection = Projection(gcs);
		return true;
	} else if (name == "Mercator")
		pcs = PCS(gcs, 1024, setup, 9001);
	else if (name == "Transverse Mercator"
	  || name == "(UTM) Universal Transverse Mercator")
		pcs = PCS(gcs, 9807, setup, 9001);
	else if (name == "Lambert Conformal Conic")
		pcs = PCS(gcs, 9802, setup, 9001);
	else if (name == "Albers Equal Area")
		pcs = PCS(gcs, 9822, setup, 9001);
	else if (name == "(A)Lambert Azimuthual Equal Area")
		pcs = PCS(gcs, 9820, setup, 9001);
	else if (name == "Polyconic (American)")
		pcs = PCS(gcs, 9818, setup, 9001);
	else if (name == "(NZTM2) New Zealand TM 2000")
		pcs = PCS(gcs, 9807, Projection::Setup(0, 173.0, 0.9996, 1600000,
		  10000000, NAN, NAN), 9001);
	else if (name == "(BNG) British National Grid")
		pcs = PCS(gcs, 9807, Projection::Setup(49, -2, 0.999601, 400000,
		  -100000, NAN, NAN), 9001);
	else if (name == "(IG) Irish Grid")
		pcs = PCS(gcs, 9807, Projection::Setup(53.5, -8, 1.000035, 200000,
		  250000, NAN, NAN), 9001);
	else if (name == "(SG) Swedish Grid")
		pcs = PCS(gcs, 9807, Projection::Setup(0, 15.808278, 1, 1500000, 0, NAN,
		  NAN), 9001);
	else if (name == "(I) France Zone I")
		pcs = PCS(gcs, 9802, Projection::Setup(49.5, 2.337229, NAN, 600000,
		  1200000, 48.598523, 50.395912), 9001);
	else if (name == "(II) France Zone II")
		pcs = PCS(gcs, 9802, Projection::Setup(46.8, 2.337229, NAN, 600000,
		  2200000, 45.898919, 47.696014), 9001);
	else if (name == "(III) France Zone III")
		pcs = PCS(gcs, 9802, Projection::Setup(44.1, 2.337229, NAN, 600000,
		  3200000, 43.199291, 44.996094), 9001);
	else if (name == "(IV) France Zone IV")
		pcs = PCS(gcs, 9802, Projection::Setup(42.165, 2.337229, NAN, 234.358,
		  4185861.369, 41.560388, 42.767663), 9001);
	else if (name == "(VICGRID) Victoria Australia")
		pcs = PCS(gcs, 9802, Projection::Setup(-37, 145, NAN, 2500000, 4500000,
		  -36, -38), 9001);
	else if (name == "(VG94) VICGRID94 Victoria Australia")
		pcs = PCS(gcs, 9802, Projection::Setup(-37, 145, NAN, 2500000, 2500000,
		  -36, -38), 9001);
	else if (name == "(SUI) Swiss Grid")
		pcs = PCS(gcs, 9815, Projection::Setup(46.570866, 7.26225, 1.0, 600000,
		  200000, 90.0, 90.0), 9001);
	else {
		_errorString = QString("%1: Unknown map projection").arg(name);
		return false;
	}

	_projection = Projection(pcs);

	return true;
}

bool MapFile::computeTransformation(const QList<CalibrationPoint> &points)
{
	QList<ReferencePoint> rp;

	for (int i = 0; i < points.size(); i++)
		rp.append(points.at(i).rp(_projection));

	_transform = Transform(rp);
	if (!_transform.isValid()) {
		_errorString = _transform.errorString();
		return false;
	}

	return true;
}

MapFile::MapFile(QIODevice &file)
{
	QList<CalibrationPoint> points;
	QString ct, datum;
	Projection::Setup setup;

	if (!parseMapFile(file, points, ct, setup, datum))
		return;
	if (!createProjection(datum, ct, setup))
		return;
	if (!computeTransformation(points))
		return;
}
