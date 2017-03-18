#include <QtGlobal>
#include <QPainter>
#include <QFileInfo>
#include <QMap>
#include <QDir>
#include "misc.h"
#include "rd.h"
#include "wgs84.h"
#include "coordinates.h"
#include "matrix.h"
#include "ozimap.h"


int OziMap::parseMapFile(QIODevice &device, QList<ReferencePoint> &points)
{
	bool res;
	int ln = 1;


	if (!device.open(QIODevice::ReadOnly))
		return -1;

	while (!device.atEnd()) {
		QByteArray line = device.readLine();

		if (ln == 1) {
			if (line.trimmed() != "OziExplorer Map Data File Version 2.2")
				return ln;
		} else if (ln == 2)
			_name = line.trimmed();
		else if (ln == 3)
			_imgPath = line.trimmed();
		else if (ln >= 10 && ln < 40) {
			QList<QByteArray> list = line.split(',');
			if (list.count() == 17 && !list.at(2).trimmed().isEmpty()) {
				int x = list.at(2).trimmed().toInt(&res);
				if (!res)
					return ln;
				int y = list.at(3).trimmed().toInt(&res);
				if (!res)
					return ln;
				int latd = list.at(6).trimmed().toInt(&res);
				if (!res)
					return ln;
				qreal latm = list.at(7).trimmed().toFloat(&res);
				if (!res)
					return ln;
				int lond = list.at(9).trimmed().toInt(&res);
				if (!res)
					return ln;
				qreal lonm = list.at(10).trimmed().toFloat(&res);
				if (!res)
					return ln;

				if (list.at(8).trimmed() == "S")
					latd = -latd;
				if (list.at(11).trimmed() == "W")
					lond = -lond;
				points.append(QPair<QPoint, Coordinates>(QPoint(x, y),
				  Coordinates(lond + lonm/60.0, latd + latm/60.0)));
			}
		} else {
			QList<QByteArray> list = line.split(',');
			if (list.at(0).trimmed() == "IWH") {
				int w = list.at(2).trimmed().toInt(&res);
				if (!res)
					return ln;
				int h = list.at(3).trimmed().toInt(&res);
				if (!res)
					return ln;
				_size = QSize(w, h);
			}
		}

		ln++;
	}

	return 0;
}

bool OziMap::computeTransformation(const QList<ReferencePoint> &points)
{
	if (points.count() < 2)
		return false;

	Matrix c(3, 2);
	c.zeroize();
	for (size_t j = 0; j < c.w(); j++) {
		for (size_t k = 0; k < c.h(); k++) {
			for (int i = 0; i < points.size(); i++) {
				double f[3], t[2];

				f[0] = points.at(i).second.lon();
				f[1] = points.at(i).second.lat();
				f[2] = 1.0;
				t[0] = points.at(i).first.x();
				t[1] = points.at(i).first.y();
				c.m(k,j) += f[k] * t[j];
			}
		}
	}

	Matrix Q(3, 3);
	Q.zeroize();
	for (int qi = 0; qi < points.size(); qi++) {
		double v[3];

		v[0] = points.at(qi).second.lon();
		v[1] = points.at(qi).second.lat();
		v[2] = 1.0;
		for (size_t i = 0; i < Q.h(); i++)
			for (size_t j = 0; j < Q.w(); j++)
				Q.m(i,j) += v[i] * v[j];
	}

	Matrix M = Q.augemented(c);
	if (!M.eliminate())
		return false;

	_transform = QTransform(M.m(0,3), M.m(1,3), M.m(0,4), M.m(1,4),
	  M.m(2,3), M.m(2,4));

	return true;
}

bool OziMap::computeResolution(QList<ReferencePoint> &points)
{
	if (points.count() < 2)
		return false;

	int maxLon = 0, minLon = 0, maxLat = 0, minLat = 0;
	qreal dLon, pLon, dLat, pLat;

	for (int i = 1; i < points.size(); i++) {
		if (points.at(i).second.lon() < points.at(minLon).second.lon())
			minLon = i;
		if (points.at(i).second.lon() > points.at(maxLon).second.lon())
			maxLon = i;
		if (points.at(i).second.lat() < points.at(minLat).second.lon())
			minLat = i;
		if (points.at(i).second.lat() > points.at(maxLat).second.lon())
			maxLat = i;
	}

	dLon = points.at(minLon).second.distanceTo(points.at(maxLon).second);
	pLon = QLineF(points.at(minLon).first, points.at(maxLon).first).length();
	dLat = points.at(minLat).second.distanceTo(points.at(maxLat).second);
	pLat = QLineF(points.at(minLat).first, points.at(maxLat).first).length();

	_resolution = (dLon/pLon + dLat/pLat) / 2.0;

	return true;
}

OziMap::OziMap(const QString &path, QObject *parent) : Map(parent)
{
	int errorLine;
	QList<ReferencePoint> points;


	_valid = false;

	QFileInfo fi(path);
	_name = fi.baseName();

	QDir dir(path);
	QFileInfoList list = dir.entryInfoList(QStringList("*.map"), QDir::Files);
	if (!list.count()) {
		qWarning("%s: map file not found", qPrintable(_name));
		return;
	}

	QFile mapFile(list[0].absoluteFilePath());
	if ((errorLine = parseMapFile(mapFile, points))) {
		if (errorLine < 0)
			qWarning("%s: error opening map file", qPrintable(_name));
		else
			qWarning("%s: map file parse error on line: %d", qPrintable(_name),
			  errorLine);
		return;
	}

	if (!computeTransformation(points)) {
		qWarning("%s: error computing map transformation", qPrintable(_name));
		return;
	}
	computeResolution(points);

	QFileInfo ii(_imgPath);
	if (ii.isRelative())
		_imgPath = fi.absoluteFilePath() + "/" + _imgPath;
	ii = QFileInfo(_imgPath);
	if (!ii.exists()) {
		qWarning("%s: %s: no such image", qPrintable(_name),
		  qPrintable(ii.absoluteFilePath()));
		return;
	}
	if (_size.isNull()) {
		qWarning("%s: missing or invalid image size (IWH)", qPrintable(_name));
		return;
	}

	_img = 0;
	_valid = true;
}

void OziMap::load()
{
	Q_ASSERT(_img == 0);

	_img = new QImage(_imgPath);
	if (_img->isNull())
		qWarning("%s: error loading map image", qPrintable(_name));
}

void OziMap::unload()
{
	Q_ASSERT(_img != 0);

	delete _img;
	_img = 0;
}

QRectF OziMap::bounds() const
{
	return QRectF(QPointF(0, 0), _size);
}

qreal OziMap::zoomFit(const QSize &size, const QRectF &br)
{
	Q_UNUSED(size);
	Q_UNUSED(br);

	return 1.0;
}

qreal OziMap::resolution(const QPointF &p) const
{
	Q_UNUSED(p);

	return _resolution;
}

qreal OziMap::zoomIn()
{
	return 1.0;
}

qreal OziMap::zoomOut()
{
	return 1.0;
}

void OziMap::draw(QPainter *painter, const QRectF &rect)
{
	Q_ASSERT(_img != 0);

	QPoint p = rect.topLeft().toPoint();
	QImage crop = _img->copy(QRect(p, rect.size().toSize()));
	painter->drawImage(rect.topLeft(), crop);
}

QPointF OziMap::ll2xy(const Coordinates &c) const
{
	return _transform.map(QPointF(c.lon(), c.lat()));
}

Coordinates OziMap::xy2ll(const QPointF &p) const
{
	return _transform.inverted().map(p);
}
