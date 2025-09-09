#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QImageReader>
#include "image.h"
#include "prjfile.h"
#include "wldfile.h"
#include "worldfilemap.h"


WorldFileMap::WorldFileMap(const QString &fileName, const Projection &proj,
  QObject *parent) : Map(fileName, parent), _img(0), _mapRatio(1.0),
  _hasPRJ(false), _valid(false)
{
	QFileInfo fi(fileName);
	QDir dir(fi.absoluteDir());
	QString basename(fi.completeBaseName());


	// Get the transformation from the WLD file
	WLDFile wld(fileName);
	if (wld.transform().isValid())
		_transform = wld.transform();
	else {
		_errorString = wld.errorString();
		return;
	}

	// Get the projection from the corresponding PRJ file (if any)
	QString prjFile(basename + ".prj");
	if (dir.exists(prjFile)) {
		PRJFile prj(dir.filePath(prjFile));
		if (prj.projection().isValid()) {
			_projection = prj.projection();
			_hasPRJ = true;
		} else {
			_errorString = prjFile + ": " + prj.errorString();
			return;
		}
	} else
		_projection = proj;

	// Find the corresponding image file
	QList<QByteArray> formats(QImageReader::supportedImageFormats());
	QString imgFile;

	for (int i = 0; i < formats.size(); i++) {
		imgFile = basename + "." + formats.at(i);
		if (dir.exists(imgFile)) {
			_imgFile = dir.filePath(imgFile);
			break;
		}
	}

	if (_imgFile.isNull()) {
		_errorString = "image file not found";
		return;
	}

	QImageReader ir(_imgFile);
	if (!ir.canRead()) {
		_errorString = imgFile + ": unsupported/invalid image file";
		return;
	}
	_size = ir.size();

	_valid = true;
}

WorldFileMap::~WorldFileMap()
{
	delete _img;
}

QPointF WorldFileMap::ll2xy(const Coordinates &c)
{
	return QPointF(_transform.proj2img(_projection.ll2xy(c))) / _mapRatio;
}

Coordinates WorldFileMap::xy2ll(const QPointF &p)
{
	return _projection.xy2ll(_transform.img2proj(p * _mapRatio));
}

QRectF WorldFileMap::bounds()
{
	return QRectF(QPointF(0, 0), _size / _mapRatio);
}

void WorldFileMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	if (_img)
		_img->draw(painter, rect, flags);
}

void WorldFileMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi, int layer)
{
	Q_UNUSED(out);
	Q_UNUSED(layer);

	if (!_hasPRJ)
		_projection = in;

	_mapRatio = hidpi ? deviceRatio : 1.0;

	Q_ASSERT(!_img);
	_img = new Image(_imgFile);
	if (_img)
		_img->setDevicePixelRatio(_mapRatio);
}

void WorldFileMap::unload()
{
	delete _img;
	_img = 0;
}

Map *WorldFileMap::create(const QString &path, const Projection &proj,
  bool *isDir)
{
	if (isDir)
		*isDir = false;

	return new WorldFileMap(path, proj);
}
