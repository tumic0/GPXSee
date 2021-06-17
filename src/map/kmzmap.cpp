/*
	WARNING: This code uses internal Qt API - the QZipReader class for reading
	ZIP files - and things may break if Qt changes the API. For Qt5 this is not
	a problem as we can "see the future" now and there are no changes in all
	the supported Qt5 versions up to the last one (5.15). In Qt6 the class
	might change or even disappear in the future, but this is very unlikely
	as there were no changes for several years and The Qt Company's policy
	is: "do not invest any resources into any desktop related stuff unless
	absolutely necessary". There is an issue (QTBUG-3897) since the year 2009 to
	include the ZIP reader into the public API, which aptly illustrates the
	effort The Qt Company is willing to make about anything desktop related...
*/

#include <QFileInfo>
#include <QXmlStreamReader>
#include <QImage>
#include <QImageReader>
#include <QBuffer>
#include <QPainter>
#include <private/qzipreader_p.h>
#include"common/util.h"
#include "pcs.h"
#include "image.h"
#include "kmzmap.h"


#define ZOOM_THRESHOLD 0.9

#define TL(m) ((m).bbox().topLeft())
#define BR(m) ((m).bbox().bottomRight())


KMZMap::Overlay::Overlay(const QString &path, const QSize &size,
  const RectC &bbox, double rotation, const Projection *proj, qreal ratio)
  : _path(path), _size(size), _bbox(bbox), _rotation(rotation), _img(0),
  _proj(proj), _ratio(ratio)
{
	ReferencePoint tl(PointD(0, 0), _proj->ll2xy(bbox.topLeft()));
	ReferencePoint br(PointD(size.width(), size.height()),
	  _proj->ll2xy(bbox.bottomRight()));

	QTransform t;
	t.rotate(-rotation);
	QRectF b(0, 0, size.width(), size.height());
	QPolygonF ma = t.map(b);
	_bounds = ma.boundingRect();

	_transform = Transform(tl, br);
}

qreal KMZMap::Overlay::resolution(const QRectF &rect) const
{
	qreal cy = rect.center().y();
	QPointF cl(rect.left(), cy);
	QPointF cr(rect.right(), cy);

	qreal ds = xy2ll(cl).distanceTo(xy2ll(cr));
	qreal ps = QLineF(cl, cr).length();

	return ds/ps;
}

void KMZMap::Overlay::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	if (_img) {
		QRectF rr(rect.topLeft() / _ratio, rect.size());

		if (_rotation) {
			painter->save();
			painter->rotate(-_rotation);
			_img->draw(painter, rr, flags);
			painter->restore();
		} else
			_img->draw(painter, rr, flags);
	}

	//painter->setPen(Qt::red);
	//painter->drawRect(_bounds);
}

void KMZMap::Overlay::load(QZipReader *zip)
{
	if (!_img) {
		QByteArray ba(zip->fileData(_path));
		_img = new Image(QImage::fromData(ba));
		_img->setDevicePixelRatio(_ratio);
	}
}

void KMZMap::Overlay::unload()
{
	delete _img;
	_img = 0;
}

void KMZMap::Overlay::setProjection(const Projection *proj)
{
	_proj = proj;

	ReferencePoint tl(PointD(0, 0), _proj->ll2xy(_bbox.topLeft()));
	ReferencePoint br(PointD(_size.width(), _size.height()),
	  _proj->ll2xy(_bbox.bottomRight()));

	QTransform t;
	t.rotate(-_rotation);
	QRectF b(0, 0, _size.width(), _size.height());
	QPolygonF ma = t.map(b);
	_bounds = ma.boundingRect();

	_transform = Transform(tl, br);
}

void KMZMap::Overlay::setDevicePixelRatio(qreal ratio)
{
	_ratio = ratio;

	if (_img)
		_img->setDevicePixelRatio(_ratio);
}


bool KMZMap::resCmp(const Overlay &m1, const Overlay &m2)
{
	qreal r1, r2;

	r1 = m1.resolution(m1.bounds());
	r2 = m2.resolution(m2.bounds());

	return r1 > r2;
}

bool KMZMap::xCmp(const Overlay &m1, const Overlay &m2)
{
	return TL(m1).lon() < TL(m2).lon();
}

bool KMZMap::yCmp(const Overlay &m1, const Overlay &m2)
{
	return TL(m1).lat() > TL(m2).lat();
}

void KMZMap::computeZooms()
{
	std::sort(_maps.begin(), _maps.end(), resCmp);

	_zooms.append(Zoom(0, _maps.count() - 1));
	for (int i = 1; i < _maps.count(); i++) {
		qreal last = _maps.at(i-1).resolution(_maps.at(i).bounds());
		qreal cur = _maps.at(i).resolution(_maps.at(i).bounds());
		if (cur < last * ZOOM_THRESHOLD) {
			_zooms.last().last = i-1;
			_zooms.append(Zoom(i, _maps.count() - 1));
		}
	}
}

void KMZMap::computeBounds()
{
	QVector<QPointF> offsets(_maps.count());

	for (int z = 0; z < _zooms.count(); z++) {
		QList<Overlay> m;
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).last; i++)
			m.append(_maps.at(i));

		std::sort(m.begin(), m.end(), xCmp);
		offsets[_maps.indexOf(m.first())].setX(m.first().bounds().left());
		for (int i = 1; i < m.size(); i++) {
			qreal w = m.first().ll2xy(TL(m.at(i))).x();
			offsets[_maps.indexOf(m.at(i))].setX(w + m.at(i).bounds().left());
		}

		std::sort(m.begin(), m.end(), yCmp);
		offsets[_maps.indexOf(m.first())].setY(m.first().bounds().top());
		for (int i = 1; i < m.size(); i++) {
			qreal h = m.first().ll2xy(TL(m.at(i))).y();
			offsets[_maps.indexOf(m.at(i))].setY(h + m.at(i).bounds().top());
		}
	}

	_adjust = 0;
	_bounds = QVector<Bounds>(_maps.count());
	for (int i = 0; i < _maps.count(); i++) {
		QRectF xy(offsets.at(i), _maps.at(i).bounds().size());
		_bounds[i] = Bounds(_maps.at(i).bbox(), xy);
		_adjust = qMin(qMin(_maps.at(i).bounds().left(),
		  _maps.at(i).bounds().top()), _adjust);
	}
	_adjust = -_adjust;
}


double KMZMap::number(QXmlStreamReader &reader)
{
	bool res;
	double ret = reader.readElementText().toDouble(&res);
	if (!res)
		reader.raiseError(QString("Invalid %1").arg(reader.name().toString()));

	return ret;
}

QString KMZMap::icon(QXmlStreamReader &reader)
{
	QString href;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("href"))
			href = reader.readElementText();
		else
			reader.skipCurrentElement();
	}

	return href;
}

RectC KMZMap::latLonBox(QXmlStreamReader &reader, double *rotation)
{
	double top = NAN, bottom = NAN, right = NAN, left = NAN;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("north"))
			top = number(reader);
		else if (reader.name() == QLatin1String("south"))
			bottom = number(reader);
		else if (reader.name() == QLatin1String("west"))
			left = number(reader);
		else if (reader.name() == QLatin1String("east"))
			right = number(reader);
		else if (reader.name() == QLatin1String("rotation"))
			*rotation = number(reader);
		else
			reader.skipCurrentElement();
	}

	return RectC(Coordinates(left, top), Coordinates(right, bottom));
}

void KMZMap::groundOverlay(QXmlStreamReader &reader, QZipReader &zip)
{
	QString image;
	RectC rect;
	double rotation = 0;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Icon"))
			image = icon(reader);
		else if (reader.name() == QLatin1String("LatLonBox"))
			rect = latLonBox(reader, &rotation);
		else
			reader.skipCurrentElement();
	}

	if (rect.isValid()) {
		QByteArray ba(zip.fileData(image));
		QBuffer img(&ba);
		QImageReader ir(&img);
		QSize size(ir.size());

		if (size.isValid())
			_maps.append(Overlay(image, size, rect, rotation, &_projection,
			  _ratio));
		else
			reader.raiseError(image + ": Invalid image file");
	} else
		reader.raiseError("Invalid LatLonBox");
}

void KMZMap::document(QXmlStreamReader &reader, QZipReader &zip)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Document"))
			document(reader, zip);
		else if (reader.name() == QLatin1String("GroundOverlay"))
			groundOverlay(reader, zip);
		else if (reader.name() == QLatin1String("Folder"))
			folder(reader, zip);
		else
			reader.skipCurrentElement();
	}
}

void KMZMap::folder(QXmlStreamReader &reader, QZipReader &zip)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("GroundOverlay"))
			groundOverlay(reader, zip);
		else if (reader.name() == QLatin1String("Folder"))
			folder(reader, zip);
		else
			reader.skipCurrentElement();
	}
}

void KMZMap::kml(QXmlStreamReader &reader, QZipReader &zip)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Document"))
			document(reader, zip);
		else if (reader.name() == QLatin1String("GroundOverlay"))
			groundOverlay(reader, zip);
		else if (reader.name() == QLatin1String("Folder"))
			folder(reader, zip);
		else
			reader.skipCurrentElement();
	}
}


KMZMap::KMZMap(const QString &fileName, const Projection &proj, QObject *parent)
  : Map(fileName, parent), _zoom(0), _mapIndex(-1), _zip(0), _projection(proj),
  _ratio(1.0), _valid(false)
{
	QZipReader zip(fileName, QIODevice::ReadOnly);
	QByteArray xml(zip.fileData("doc.kml"));
	QXmlStreamReader reader(xml);

	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("kml"))
			kml(reader, zip);
		else
			reader.raiseError("Not a KMZ file");
	}

	if (reader.error()) {
		_errorString = "doc.kml:" + QString::number(reader.lineNumber()) + ": "
		  + reader.errorString();
		return;
	}
	if (_maps.isEmpty()) {
		_errorString = "No usable GroundOverlay found";
		return;
	}

	computeZooms();
	computeBounds();

	_valid = true;
}

QRectF KMZMap::bounds()
{
	QRectF rect;

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++)
		rect |= _bounds.at(i).xy;

	rect.moveTopLeft(rect.topLeft() * 2);
	return rect;
}

int KMZMap::zoomFit(const QSize &size, const RectC &br)
{
	_zoom = 0;
	_mapIndex = -1;

	if (!br.isValid()) {
		_zoom = _zooms.size() - 1;
		return _zoom;
	}

	for (int z = 0; z < _zooms.count(); z++) {
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).last; i++) {
			if (!_bounds.at(i).ll.contains(br.center()))
				continue;

			QRect sbr = QRectF(_maps.at(i).ll2xy(br.topLeft()),
			  _maps.at(i).ll2xy(br.bottomRight())).toRect().normalized();

			if (sbr.size().width() > size.width()
			  || sbr.size().height() > size.height())
				return _zoom;

			_zoom = z;
			break;
		}
	}

	return _zoom;
}

void KMZMap::setZoom(int zoom)
{
	_mapIndex = -1;
	_zoom = zoom;
}

int KMZMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	_mapIndex = -1;

	return _zoom;
}

int KMZMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	_mapIndex = -1;

	return _zoom;
}

QPointF KMZMap::ll2xy(const Coordinates &c)
{
	if (_mapIndex < 0 || !_bounds.at(_mapIndex).ll.contains(c)) {
		_mapIndex = _zooms.at(_zoom).first;
		for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
			if (_bounds.at(i).ll.contains(c)) {
				_mapIndex = i;
				break;
			}
		}
	}

	QPointF p = _maps.at(_mapIndex).ll2xy(c);
	if (_maps.at(_mapIndex).rotation()) {
		QTransform matrix;
		matrix.rotate(-_maps.at(_mapIndex).rotation());
		return matrix.map(p) + _bounds.at(_mapIndex).xy.topLeft();
	} else
		return p + _bounds.at(_mapIndex).xy.topLeft();
}

Coordinates KMZMap::xy2ll(const QPointF &p)
{
	int idx = _zooms.at(_zoom).first;

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
		if (_bounds.at(i).xy.contains(p)) {
			idx = i;
			break;
		}
	}

	QPointF p2 = p - _bounds.at(idx).xy.topLeft();
	if (_maps.at(idx).rotation()) {
		QTransform matrix;
		matrix.rotate(_maps.at(idx).rotation());
		return _maps.at(idx).xy2ll(matrix.map(p2));
	} else
		return _maps.at(idx).xy2ll(p2);
}

void KMZMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	QRectF er = rect.adjusted(-_adjust * _ratio, -_adjust * _ratio,
	  _adjust * _ratio, _adjust * _ratio);

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
		QRectF ir = er.intersected(_bounds.at(i).xy);
		if (!ir.isNull())
			draw(painter, ir, i, flags);
	}
}

void KMZMap::load()
{
	Q_ASSERT(!_zip);
	_zip = new QZipReader(path(), QIODevice::ReadOnly);
}

void KMZMap::unload()
{
	for (int i = 0; i < _maps.count(); i++)
		_maps[i].unload();

	delete _zip;
	_zip = 0;
}

void KMZMap::setInputProjection(const Projection &projection)
{
	if (projection == _projection)
		return;

	_projection = projection;

	for (int i = 0; i < _maps.size(); i++)
		_maps[i].setProjection(&_projection);

	_bounds.clear();
	computeBounds();
}

void KMZMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(deviceRatio);

	if (mapRatio == _ratio)
		return;

	_ratio = mapRatio;

	for (int i = 0; i < _maps.size(); i++)
		_maps[i].setDevicePixelRatio(_ratio);

	_bounds.clear();
	computeBounds();
}

void KMZMap::draw(QPainter *painter, const QRectF &rect, int mapIndex,
  Flags flags)
{
	Overlay &map = _maps[mapIndex];
	const QPointF offset = _bounds.at(mapIndex).xy.topLeft();
	QRectF pr = QRectF(rect.topLeft() - offset, rect.size());

	map.load(_zip);

	painter->save();
	painter->translate(offset);
	map.draw(painter, pr, flags);
	painter->restore();
}
