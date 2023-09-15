#include <QDir>
#include <QtAlgorithms>
#include <QPainter>
#include "common/rectc.h"
#include "ozimap.h"
#include "tar.h"
#include "atlas.h"


#define ZOOM_THRESHOLD 0.8

#define TL(m) ((m)->xy2pp((m)->bounds().topLeft()))
#define BR(m) ((m)->xy2pp((m)->bounds().bottomRight()))

static bool resCmp(OziMap *m1, OziMap *m2)
{
	return m1->resolution(m1->bounds()) > m2->resolution(m2->bounds());
}

static bool xCmp(OziMap *m1, OziMap *m2)
{
	return TL(m1).x() < TL(m2).x();
}

static bool yCmp(OziMap *m1, OziMap *m2)
{
	return TL(m1).y() > TL(m2).y();
}

static QString calibrationFile(const QString &path)
{
	QDir dir(path);
	QFileInfoList files = dir.entryInfoList(QDir::Files);

	for (int i = 0; i < files.size(); i++) {
		const QFileInfo &fi = files.at(i);
		QString suffix(fi.suffix().toLower());

		if (suffix == "map" || suffix == "gmi")
			return fi.absoluteFilePath();
	}

	return QString();
}

static QString tbaFile(const QStringList &files)
{
	for (int i = 0; i < files.size(); i++) {
		QFileInfo fi(files.at(i));

		if (fi.path() == "." && fi.suffix().toLower() == "tba")
			return files.at(i);
	}

	return QString();
}

void Atlas::computeZooms()
{
	std::sort(_maps.begin(), _maps.end(), resCmp);

	_zooms.append(Zoom(0, _maps.count() - 1));
	for (int i = 1; i < _maps.count(); i++) {
		qreal last = _maps.at(i-1)->resolution(_maps.at(i-1)->bounds());
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

Atlas::Atlas(const QString &fileName, bool TAR, const Projection &proj,
  QObject *parent) : Map(fileName, parent), _zoom(0), _mapIndex(-1),
  _valid(false)
{
	QFileInfo fi(fileName);
	QByteArray ba;
	Tar tar(fileName);

	_name = fi.dir().dirName();

	if (TAR) {
		if (!tar.open()) {
			_errorString = "Error reading tar file";
			return;
		}
		QString tbaFileName(tbaFile(tar.files()));
		if (tbaFileName.isNull()) {
			_errorString = "No tba file found";
			return;
		}
		ba = tar.file(tbaFileName);
	} else {
		QFile tbaFile(fileName);
		if (!tbaFile.open(QIODevice::ReadOnly)) {
			_errorString = tbaFile.errorString();
			return;
		}
		ba = tbaFile.readAll();
	}
	if (!ba.startsWith("Atlas 1.0")) {
		_errorString = "Invalid tba file";
		return;
	}

	QDir dir(fi.absolutePath());
	QFileInfoList layers = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int n = 0; n < layers.count(); n++) {
		QDir zdir(layers.at(n).absoluteFilePath());
		QFileInfoList maps = zdir.entryInfoList(QDir::Dirs
		  | QDir::NoDotAndDotDot);
		for (int i = 0; i < maps.count(); i++) {
			OziMap *map;
			if (TAR)
				map = new OziMap(maps.at(i).absoluteFilePath(), tar, proj, this);
			else {
				QString cf(calibrationFile(maps.at(i).absoluteFilePath()));
				if (cf.isNull()) {
					_errorString = "No calibration file found";
					return;
				}
				map = new OziMap(cf, proj, this);
			}

			if (map->isValid())
				_maps.append(map);
			else {
				qWarning("%s: %s",  qPrintable(map->path()),
				  qPrintable(map->errorString()));
				delete map;
			}
		}
	}
	if (_maps.isEmpty()) {
		_errorString = "No usable map found in atlas";
		return;
	}

	_valid = true;
}

RectC Atlas::llBounds()
{
	RectC bounds;

	for (int i = 0; i < _maps.size(); i++)
		bounds |= _maps.at(i)->llBounds();

	return bounds;
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

	painter->translate(offset);
	map->draw(painter, pr, flags);
	painter->translate(-offset);
}

void Atlas::load(const Projection &in, const Projection &out, qreal deviceRatio,
  bool hidpi)
{
	for (int i = 0; i < _maps.count(); i++)
		_maps.at(i)->load(in, out, deviceRatio, hidpi);

	computeZooms();
	computeBounds();
}

void Atlas::unload()
{
	for (int i = 0; i < _maps.count(); i++)
		_maps.at(i)->unload();

	_zooms.clear();
	_bounds.clear();
}

Map *Atlas::createTAR(const QString &path, const Projection &proj, bool *isDir)
{
	if (isDir)
		*isDir = true;

	return new Atlas(path, true, proj);
}

Map *Atlas::createTBA(const QString &path, const Projection &proj, bool *isDir)
{
	if (isDir)
		*isDir = true;

	return new Atlas(path, false, proj);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Atlas::Bounds &bounds)
{
	dbg.nospace() << "Bounds(" << bounds.xy << ", " << bounds.pp << ")";

	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Atlas::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.first << ", " << zoom.last << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
