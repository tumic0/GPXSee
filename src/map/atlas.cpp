#include <QDir>
#include <QtAlgorithms>
#include <QPainter>
#include "common/rectc.h"
#include "ozimap.h"
#include "tar.h"
#include "atlas.h"


#define ZOOM_THRESHOLD 0.9

#define TL(m) ((m)->xy2pp((m)->bounds().topLeft()))
#define BR(m) ((m)->xy2pp((m)->bounds().bottomRight()))

static bool resCmp(OziMap *m1, OziMap *m2)
{
	qreal r1, r2;

	r1 = m1->resolution(m1->bounds());
	r2 = m2->resolution(m2->bounds());

	return r1 > r2;
}

static bool xCmp(OziMap *m1, OziMap *m2)
{
	return TL(m1).x() < TL(m2).x();
}

static bool yCmp(OziMap *m1, OziMap *m2)
{
	return TL(m1).y() > TL(m2).y();
}

void Atlas::computeZooms()
{
	std::sort(_maps.begin(), _maps.end(), resCmp);

	_zooms.append(Zoom(0, _maps.count() - 1));
	for (int i = 1; i < _maps.count(); i++) {
		qreal last = _maps.at(i-1)->resolution(_maps.at(i)->bounds());
		qreal cur = _maps.at(i)->resolution(_maps.at(i)->bounds());
		if (cur < last * ZOOM_THRESHOLD) {
			_zooms.last().last = i-1;
			_zooms.append(Zoom(i, _maps.count() - 1));
		}
	}
}

void Atlas::computeBounds()
{
	QVector<QPointF> offsets(_maps.count());

	for (int z = 0; z < _zooms.count(); z++) {
		QList<OziMap*> m;
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).last; i++)
			m.append(_maps.at(i));

		std::sort(m.begin(), m.end(), xCmp);
		offsets[_maps.indexOf(m.first())].setX(0);
		for (int i = 1; i < m.size(); i++) {
			qreal w = m.first()->pp2xy(TL(m.at(i))).x();
			offsets[_maps.indexOf(m.at(i))].setX(w);
		}

		std::sort(m.begin(), m.end(), yCmp);
		offsets[_maps.indexOf(m.first())].setY(0);
		for (int i = 1; i < m.size(); i++) {
			qreal h = m.first()->pp2xy(TL(m.at(i))).y();
			offsets[_maps.indexOf(m.at(i))].setY(h);
		}
	}

	_bounds = QVector<Bounds>(_maps.count());
	for (int i = 0; i < _maps.count(); i++)
		_bounds[i] = Bounds(RectD(TL(_maps.at(i)), BR(_maps.at(i))),
		  QRectF(offsets.at(i), _maps.at(i)->bounds().size()));
}

Atlas::Atlas(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _zoom(0), _mapIndex(-1), _valid(false)
{
	QFileInfo fi(fileName);
	QByteArray ba;
	QString suffix = fi.suffix().toLower();
	Tar tar(fileName);


	_name = fi.dir().dirName();

	if (suffix == "tar") {
		if (!tar.open()) {
			_errorString = "Error reading tar file";
			return;
		}
		QString tbaFileName = fi.completeBaseName() + ".tba";
		ba = tar.file(tbaFileName);
	} else if (suffix == "tba") {
		QFile tbaFile(fileName);
		if (!tbaFile.open(QIODevice::ReadOnly)) {
			_errorString = QString("Error opening tba file: %1")
			  .arg(tbaFile.errorString());
			return;
		}
		ba = tbaFile.readAll();
	}
	if (!ba.startsWith("Atlas 1.0")) {
		_errorString = "Missing or invalid tba file";
		return;
	}

	QDir dir(fi.absolutePath());
	QFileInfoList layers = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int n = 0; n < layers.count(); n++) {
		QDir zdir(layers.at(n).absoluteFilePath());
		QFileInfoList maps = zdir.entryInfoList(QDir::Dirs
		  | QDir::NoDotAndDotDot);
		for (int i = 0; i < maps.count(); i++) {
			QString mapFile = maps.at(i).absoluteFilePath() + "/"
			  + maps.at(i).fileName() + ".map";

			OziMap *map;
			if (tar.isOpen())
				map = new OziMap(mapFile, tar, this);
			else
				map = new OziMap(mapFile, this);

			if (map->isValid())
				_maps.append(map);
			else {
				_errorString = QString("Error loading map: %1: %2")
				  .arg(mapFile, map->errorString());
				return;
			}
		}
	}
	if (_maps.isEmpty()) {
		_errorString = "No maps found in atlas";
		return;
	}

	computeZooms();
	computeBounds();

	_valid = true;
}

void Atlas::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	for (int i = 0; i < _maps.size(); i++)
		_maps[i]->setDevicePixelRatio(deviceRatio, mapRatio);

	computeBounds();
}

QRectF Atlas::bounds()
{
	QSizeF s(0, 0);

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
		if (_bounds.at(i).xy.right() > s.width())
			s.setWidth(_bounds.at(i).xy.right());
		if (_bounds.at(i).xy.bottom() > s.height())
			s.setHeight(_bounds.at(i).xy.bottom());
	}

	return QRectF(QPointF(0, 0), s);
}

int Atlas::zoomFit(const QSize &size, const RectC &br)
{
	_zoom = 0;
	_mapIndex = -1;

	if (!br.isValid()) {
		_zoom = _zooms.size() - 1;
		return _zoom;
	}

	for (int z = 0; z < _zooms.count(); z++) {
		for (int i = _zooms.at(z).first; i <= _zooms.at(z).last; i++) {
			if (!_bounds.at(i).pp.contains(_maps.at(i)->ll2pp(br.center())))
				continue;

			QRect sbr = QRectF(_maps.at(i)->ll2xy(br.topLeft()),
			  _maps.at(i)->ll2xy(br.bottomRight())).toRect().normalized();

			if (sbr.size().width() > size.width()
			  || sbr.size().height() > size.height())
				return _zoom;

			_zoom = z;
			break;
		}
	}

	return _zoom;
}

void Atlas::setZoom(int zoom)
{
	_mapIndex = -1;
	_zoom = zoom;
}

int Atlas::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	_mapIndex = -1;

	return _zoom;
}

int Atlas::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	_mapIndex = -1;

	return _zoom;
}

QPointF Atlas::ll2xy(const Coordinates &c)
{
	PointD pp;

	if (_mapIndex >= 0)
		pp = _maps.at(_mapIndex)->ll2pp(c);
	if (_mapIndex < 0 || !_bounds.at(_mapIndex).pp.contains(pp)) {
		_mapIndex = _zooms.at(_zoom).first;
		for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
			pp = _maps.at(i)->ll2pp(c);
			if (_bounds.at(i).pp.contains(pp)) {
				_mapIndex = i;
				break;
			}
		}
	}

	QPointF p = _maps.at(_mapIndex)->pp2xy(pp);
	return p + _bounds.at(_mapIndex).xy.topLeft();
}

Coordinates Atlas::xy2ll(const QPointF &p)
{
	int idx = _zooms.at(_zoom).first;

	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
		if (_bounds.at(i).xy.contains(p)) {
			idx = i;
			break;
		}
	}

	QPointF p2 = p - _bounds.at(idx).xy.topLeft();
	return _maps.at(idx)->xy2ll(p2);
}

void Atlas::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	// All in one map
	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
		if (_bounds.at(i).xy.contains(rect)) {
			draw(painter, rect, i, flags);
			return;
		}
	}

	// Multiple maps
	for (int i = _zooms.at(_zoom).first; i <= _zooms.at(_zoom).last; i++) {
		QRectF ir = rect.intersected(_bounds.at(i).xy);
		if (!ir.isNull())
			draw(painter, ir, i, flags);
	}
}

void Atlas::draw(QPainter *painter, const QRectF &rect, int mapIndex,
  Flags flags)
{
	OziMap *map = _maps.at(mapIndex);
	const QPointF offset = _bounds.at(mapIndex).xy.topLeft();
	QRectF pr = QRectF(rect.topLeft() - offset, rect.size());

	map->load();

	painter->translate(offset);
	map->draw(painter, pr, flags);
	painter->translate(-offset);
}

void Atlas::unload()
{
	for (int i = 0; i < _maps.count(); i++)
		_maps.at(i)->unload();
}

bool Atlas::isAtlas(const QString &path)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();

	if (suffix == "tar") {
		Tar tar(path);
		if (!tar.open())
			return false;
		QString tbaFileName = fi.completeBaseName() + ".tba";
		return tar.contains(tbaFileName);
	} else if (suffix == "tba")
		return true;

	return false;
}
