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
#include <QBuffer>
#include <QImageReader>
#include <QPainter>
#include <QPixmapCache>
#include <private/qzipreader_p.h>
#include "common/util.h"
#include "kmzmap.h"


#define ZOOM_THRESHOLD 0.9

#define TL(m) ((m).bbox().topLeft())
#define BR(m) ((m).bbox().bottomRight())

bool KMZMap::resCmp(const Tile &m1, const Tile &m2)
{
	return m1.resolution() > m2.resolution();
}

bool KMZMap::xCmp(const Tile &m1, const Tile &m2)
{
	return TL(m1).lon() < TL(m2).lon();
}

bool KMZMap::yCmp(const Tile &m1, const Tile &m2)
{
	return TL(m1).lat() > TL(m2).lat();
}


KMZMap::Tile::Tile(const Overlay &overlay, QZipReader &zip)
  : _overlay(overlay)
{
	QByteArray ba(zip.fileData(overlay.path()));
	QBuffer img(&ba);
	QImageReader ir(&img);
	_size = ir.size();
}

void KMZMap::Tile::configure(const Projection &proj)
{
	ReferencePoint tl(PointD(0, 0), proj.ll2xy(bbox().topLeft()));
	ReferencePoint br(PointD(_size.width(), _size.height()),
	  proj.ll2xy(bbox().bottomRight()));
	_transform = Transform(tl, br);
}

QRectF KMZMap::Tile::bounds() const
{
	QTransform t;
	t.rotate(-rotation());
	QRectF b(0, 0, _size.width(), _size.height());
	QPolygonF ma(t.map(b));
	return ma.boundingRect();
}

qreal KMZMap::Tile::resolution() const
{
	QRectF d(0, 0, _size.width(), _size.height());
	qreal dy = d.center().y();
	QPointF dl(d.left(), dy);
	QPointF dr(d.right(), dy);

	double cy = bbox().center().lat();
	Coordinates cl(bbox().left(), cy);
	Coordinates cr(bbox().right(), cy);

	qreal ds = cl.distanceTo(cr);
	qreal ps = QLineF(dl, dr).length();

	return ds/ps;
}

bool KMZMap::createTiles(const QList<Overlay> &overlays, QZipReader &zip)
{
	if (overlays.isEmpty()) {
		_errorString = "No usable overlay found";
		return false;
	}

	_tiles.reserve(overlays.size());

	for (int i = 0; i < overlays.size(); i++) {
		const Overlay &ol = overlays.at(i);
		Tile tile(ol, zip);
		if (tile.isValid())
			_tiles.append(tile);
		else {
			_errorString = ol.path() + ": invalid/missing overlay image";
			return false;
		}
	}

	return true;
}

void KMZMap::computeLLBounds()
{
	for (int i = 0; i < _tiles.size(); i++)
		_llbounds |= _tiles.at(i).bbox();
}

void KMZMap::computeZooms()
{
	std::sort(_tiles.begin(), _tiles.end(), resCmp);

	_zooms.append(Zoom(0, _tiles.count() - 1));
	for (int i = 1; i < _tiles.count(); i++) {
		qreal last = _tiles.at(i-1).resolution();
		qreal cur = _tiles.at(i).resolution();
		if (cur < last * ZOOM_THRESHOLD) {
			_zooms.last().last = i-1;
			_zooms.append(Zoom(i, _tiles.count() - 1));
		}
	}
}

void KMZMap::computeBounds()
{
	QVector<QPointF> offsets(_tiles.count());

	for (int z = 0; z < _zooms.count(); z++) {
		QList<Tile> m;
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).last; i++)
			m.append(_tiles.at(i));

		std::sort(m.begin(), m.end(), xCmp);
		offsets[_tiles.indexOf(m.first())].setX(m.first().bounds().left());
		for (int i = 1; i < m.size(); i++) {
			qreal w = ll2xy(TL(m.at(i)), m.first().transform()).x();
			offsets[_tiles.indexOf(m.at(i))].setX(w + m.at(i).bounds().left());
		}

		std::sort(m.begin(), m.end(), yCmp);
		offsets[_tiles.indexOf(m.first())].setY(m.first().bounds().top());
		for (int i = 1; i < m.size(); i++) {
			qreal h = ll2xy(TL(m.at(i)), m.first().transform()).y();
			offsets[_tiles.indexOf(m.at(i))].setY(h + m.at(i).bounds().top());
		}
	}

	_adjust = 0;
	_bounds = QVector<Bounds>(_tiles.count());
	for (int i = 0; i < _tiles.count(); i++) {
		QRectF xy(offsets.at(i), _tiles.at(i).bounds().size() / _mapRatio);
		_bounds[i] = Bounds(_tiles.at(i).bbox(), xy);
		_adjust = qMin(qMin(_tiles.at(i).bounds().left(),
		  _tiles.at(i).bounds().top()), _adjust);
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

void KMZMap::groundOverlay(QXmlStreamReader &reader, QList<Overlay> &overlays)
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

	if (rect.isValid())
		overlays.append(Overlay(image, rect, rotation));
	else
		reader.raiseError("Invalid LatLonBox");
}

void KMZMap::document(QXmlStreamReader &reader, QList<Overlay> &overlays)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Document"))
			document(reader, overlays);
		else if (reader.name() == QLatin1String("GroundOverlay"))
			groundOverlay(reader, overlays);
		else if (reader.name() == QLatin1String("Folder"))
			folder(reader, overlays);
		else
			reader.skipCurrentElement();
	}
}

void KMZMap::folder(QXmlStreamReader &reader, QList<Overlay> &overlays)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("GroundOverlay"))
			groundOverlay(reader, overlays);
		else if (reader.name() == QLatin1String("Folder"))
			folder(reader, overlays);
		else
			reader.skipCurrentElement();
	}
}

void KMZMap::kml(QXmlStreamReader &reader, QList<Overlay> &overlays)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Document"))
			document(reader, overlays);
		else if (reader.name() == QLatin1String("GroundOverlay"))
			groundOverlay(reader, overlays);
		else if (reader.name() == QLatin1String("Folder"))
			folder(reader, overlays);
		else
			reader.skipCurrentElement();
	}
}


KMZMap::KMZMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _zoom(0), _mapIndex(-1), _zip(0), _mapRatio(1.0),
  _valid(false)
{
	QZipReader zip(fileName, QIODevice::ReadOnly);
	QByteArray xml(zip.fileData("doc.kml"));
	QXmlStreamReader reader(xml);
	QList<Overlay> overlays;

	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("kml"))
			kml(reader, overlays);
		else
			reader.raiseError("Not a KMZ file");
	}
	if (reader.error()) {
		_errorString = "doc.kml:" + QString::number(reader.lineNumber()) + ": "
		  + reader.errorString();
		return;
	}

	if (!createTiles(overlays, zip))
		return;
	computeLLBounds();
	computeZooms();

	_valid = true;
}

KMZMap::~KMZMap()
{
	delete _zip;
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

			QRect sbr = QRectF(ll2xy(br.topLeft(), _tiles.at(i).transform()),
			  ll2xy(br.bottomRight(), _tiles.at(i).transform()))
			  .toRect().normalized();

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

	QPointF p = ll2xy(c, _tiles.at(_mapIndex).transform());
	if (_tiles.at(_mapIndex).rotation()) {
		QTransform matrix;
		matrix.rotate(-_tiles.at(_mapIndex).rotation());
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
	if (_tiles.at(idx).rotation()) {
		QTransform matrix;
		matrix.rotate(_tiles.at(idx).rotation());
		return xy2ll(matrix.map(p2), _tiles.at(idx).transform());
	} else
		return xy2ll(p2, _tiles.at(idx).transform());
}

void KMZMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);

	QRectF er = rect.adjusted(-_adjust * _mapRatio, -_adjust * _mapRatio,
	  _adjust * _mapRatio, _adjust * _mapRatio);

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
		QRectF ir = er.intersected(_bounds.at(i).xy);
		if (!ir.isNull())
			draw(painter, ir, i);
	}
}

void KMZMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(out);

	_projection = in;
	_mapRatio = hidpi ? deviceRatio : 1.0;

	for (int i = 0; i < _tiles.size(); i++)
		_tiles[i].configure(_projection);

	computeBounds();

	Q_ASSERT(!_zip);
	_zip = new QZipReader(path(), QIODevice::ReadOnly);
}

void KMZMap::unload()
{
	_bounds.clear();

	delete _zip;
	_zip = 0;
}

void KMZMap::draw(QPainter *painter, const QRectF &rect, int mapIndex)
{
	const Tile &map = _tiles.at(mapIndex);
	const QPointF offset = _bounds.at(mapIndex).xy.topLeft();
	QRectF pr = QRectF(rect.topLeft() - offset, rect.size());
	QRectF sr(pr.topLeft() * _mapRatio, pr.size() * _mapRatio);
	QString key(path() + "/" + map.path());
	QPixmap pm;

	painter->save();
	painter->translate(offset);
	if (map.rotation())
		painter->rotate(-map.rotation());

	if (QPixmapCache::find(key, &pm)) {
		pm.setDevicePixelRatio(_mapRatio);
		painter->drawPixmap(pr.topLeft(), pm, sr);
	} else {
		QByteArray ba(_zip->fileData(map.path()));
		QImage img(QImage::fromData(ba));
		pm = QPixmap::fromImage(img);
		pm.setDevicePixelRatio(_mapRatio);
		painter->drawPixmap(pr.topLeft(), pm, sr);
		QPixmapCache::insert(key, pm);
	}

	//painter->setPen(Qt::red);
	//painter->drawRect(map.bounds());

	painter->restore();
}

Map *KMZMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new KMZMap(path);
}
