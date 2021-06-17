#include <cctype>
#include <QFileInfo>
#include <QPainter>
#include "image.h"
#include "gcs.h"
#include "pcs.h"
#include "calibrationpoint.h"
#include "color.h"
#include "bsbmap.h"


#define LINE_LIMIT 1024

static inline bool isEOH(const QByteArray &line)
{
	return (line.size() >= 2 && line.at(line.size() - 2) == 0x1A
	  && line.at(line.size() -1) == 0);
}

static inline bool isType(const QByteArray &line, const QByteArray &type)
{
	return (line.left(4) == type);
}

static inline QByteArray hdrData(const QByteArray &line)
{
	return line.right(line.size() - 4);
}

static bool readHeaderLine(QFile &file, QByteArray &line)
{
	char c;

	while (file.getChar(&c) && line.size() < LINE_LIMIT) {
		if (c == '\0') {
			line.append(c);
			return true;
		}

		if (c == '\r')
			continue;

		if (c == '\n') {
			if (!file.getChar(&c))
				return false;
			if (c == ' ') {
				do {
					if (!file.getChar(&c))
						return false;
				} while (c == ' ');
				line.append(',');
				file.ungetChar(c);
				continue;
			} else {
				file.ungetChar(c);
				return true;
			}
		}

		line.append(c);
	}

	return false;
}

static inline bool isSplitter(const QByteArray &line, int i)
{
	return (line.at(i) == ',' && line.size() - i > 3 && isupper(line.at(i+1))
	  && (isupper(line.at(i+2)) || isdigit(line.at(i+2))) && line.at(i+3) == '=');
}

static QList<QByteArray> split(const QByteArray &line)
{
	QList<QByteArray> list;
	int ls = 0;

	for (int i = 0; i < line.size(); i++) {
		if (isSplitter(line, i)) {
			list.append(line.mid(ls, i - ls));
			ls = i + 1;
		}
	}
	list.append(line.mid(ls, line.size() - ls));

	return list;
}

static QMap<QByteArray, QByteArray> kvMap(const QByteArray &line)
{
	QMap<QByteArray, QByteArray> map;
	QList<QByteArray> parts(split(line));

	for (int i = 0; i < parts.size(); i++) {
		QList<QByteArray> ba = parts.at(i).split('=');
		if (ba.size() != 2)
			continue;
		map.insert(ba.at(0), ba.at(1));
	}

	return map;
}

static double parameter(const QString &str, bool *res)
{
	if (str.isEmpty() || str == "NOT_APPLICABLE" || str == "UNKNOWN") {
		*res = true;
		return NAN;
	}

	return str.toDouble(res);
}


bool BSBMap::parseBSB(const QByteArray &line)
{
	QMap<QByteArray, QByteArray> map(kvMap(line));

	_name = QString::fromLatin1(map.value("NA"));
	if (_name.isEmpty()) {
		_errorString = "Invalid/missing BSB NA field";
		return false;
	}

	QList<QByteArray> sv(map.value("RA").split(','));
	unsigned w, h;
	bool wok = false, hok = false;
	if (sv.size() == 2) {
		w = sv.at(0).toUInt(&wok);
		h = sv.at(1).toUInt(&hok);
	}
	if (!wok || !hok || !w || !h) {
		_errorString = "Invalid BSB RA field";
		return false;
	}

	_size = QSize(w, h);

	return true;
}

bool BSBMap::parseKNP(const QByteArray &line, QString &datum, QString &proj,
  double &pp)
{
	QMap<QByteArray, QByteArray> map(kvMap(line));
	bool ok;

	if (!(map.contains("PR") && map.contains("GD") && map.contains("PP"))) {
		_errorString = "Missing KNP PR/GD/PP field";
		return false;
	}

	proj = map.value("PR");
	datum = map.value("GD");

	pp = parameter(map.value("PP"), &ok);
	if (!ok) {
		_errorString = "Invalid KNP PP field";
		return false;
	}
	_skew = parameter(map.value("SK"), &ok);
	if (!ok) {
		_errorString = "Invalid KNP SK field";
		return false;
	}

	return true;
}

bool BSBMap::parseKNQ(const QByteArray &line, double params[9])
{
	QMap<QByteArray, QByteArray> map(kvMap(line));
	bool ok;

	for (int i = 1; i <= 8; i++) {
		params[i] = parameter(map.value(QString("P%1").arg(i).toLatin1()), &ok);
		if (!ok) {
			_errorString = QString("Invalid KNQ P%1 parameter").arg(i);
			return false;
		}
	}

	return true;
}

bool BSBMap::parseREF(const QByteArray &line, const QString &datum,
  const QString &proj, double params[9], QList<ReferencePoint> &points)
{
	QList<QByteArray> fields(line.split(','));

	if (fields.size() == 5) {
		bool xok, yok, lonok, latok;
		Coordinates c(fields.at(4).toDouble(&lonok),
		  fields.at(3).toDouble(&latok));
		if (lonok && latok && c.isValid()) {
			if (_projection.isNull()) {
				if (!createProjection(datum, proj, params, c))
					return false;
			}
			CalibrationPoint p(PointD(fields.at(1).toDouble(&xok),
			  fields.at(2).toDouble(&yok)), c);
			if (xok && yok) {
				points.append(p.rp(_projection));
				return true;
			}
		}
	}

	_errorString = QString(line) + ": Invalid reference point entry";
	return false;
}

bool BSBMap::parseRGB(const QByteArray &line)
{
	QList<QByteArray> fields(line.split(','));
	bool iok, rok, gok, bok;
	int i = fields.at(0).toUInt(&iok);

	if (iok && fields.size() == 4 && i > 0 && i < 256) {
		_palette[i-1] = Color::rgb(fields.at(1).toUInt(&rok),
		  fields.at(2).toUInt(&gok), fields.at(3).toUInt(&bok));
		if (rok && gok && bok)
			return true;
	}

	_errorString = QString(line) + ": Invalid RGB entry";
	return false;
}

bool BSBMap::readHeader(QFile &file)
{
	QByteArray line;
	QString datum, proj;
	double params[9];
	QList<ReferencePoint> points;

	while (readHeaderLine(file, line)) {
		if (isEOH(line)) {
			if (!_size.isValid() || !_projection.isValid()) {
				_errorString = "Invalid KAP file header";
				return false;
			}
			return createTransform(points);
		}

		if (isType(line, "BSB/") && !parseBSB(hdrData(line)))
			return false;
		else if (isType(line, "KNP/")
		  && !parseKNP(hdrData(line), datum, proj, params[0]))
			return false;
		else if (isType(line, "KNQ/") && !parseKNQ(hdrData(line), params))
			return false;
		else if (isType(line, "REF/")
		  && !parseREF(hdrData(line), datum, proj, params, points))
			return false;
		else if (isType(line, "RGB/") && !parseRGB(hdrData(line)))
			return false;

		line.clear();
	}

	_errorString = "Not a KAP file";

	return false;
}

bool BSBMap::createTransform(QList<ReferencePoint> &points)
{
	if (_skew > 0.0 && _skew < 360.0) {
		QTransform matrix;
		matrix.rotate(-_skew);
		QTransform t(QImage::trueMatrix(matrix, _size.width(), _size.height()));

		for (int i = 0; i < points.size(); i++)
			points[i].setXY(t.map(points.at(i).xy().toPointF()));

		QPolygonF a(QRectF(0, 0, _size.width(), _size.height()));
		a = t.map(a);
		_skewSize = a.boundingRect().toAlignedRect().size();
	}

	_transform = Transform(points);
	if (!_transform.isValid()) {
		_errorString = _transform.errorString();
		return false;
	}

	return true;
}

bool BSBMap::createProjection(const QString &datum, const QString &proj,
  double params[9], const Coordinates &c)
{
	GCS gcs;
	PCS pcs;

	if (datum.isEmpty())
		gcs = GCS::gcs(4326);
	else
		gcs = GCS::gcs(datum);
	if (gcs.isNull()) {
		_errorString = datum + ": Unknown datum";
		return false;
	}

	if (!proj.compare("MERCATOR", Qt::CaseInsensitive)) {
		Projection::Setup setup(0, c.lon(), NAN, 0, 0, NAN, NAN);
		pcs = PCS(gcs, 9804, setup, 9001);
	} else if (!proj.compare("TRANSVERSE MERCATOR", Qt::CaseInsensitive)) {
		Projection::Setup setup(0, params[1], params[2], 0, 0, NAN, NAN);
		pcs = PCS(gcs, 9807, setup, 9001);
	} else if (!proj.compare("UNIVERSAL TRANSVERSE MERCATOR",
	  Qt::CaseInsensitive)) {
		Projection::Setup setup(0, params[0], 0.9996, 500000, 0, NAN, NAN);
		pcs = PCS(gcs, 9807, setup, 9001);
	} else if (!proj.compare("LAMBERT CONFORMAL CONIC", Qt::CaseInsensitive)) {
		Projection::Setup setup(0, params[0], NAN, 0, 0, params[2], params[3]);
		pcs = PCS(gcs, 9802, setup, 9001);
	} else if (!proj.compare("POLYCONIC", Qt::CaseInsensitive)) {
		Projection::Setup setup(0, params[0], NAN, 0, 0, NAN, NAN);
		pcs = PCS(gcs, 9818, setup, 9001);
	} else {
		_errorString = proj + ": Unknown/missing projection";
		return false;
	}

	_projection = Projection(pcs);

	return true;
}

bool BSBMap::readRow(QFile &file, char bits, uchar *buf)
{
	char c;
	int multiplier;
	int pixel = 1, written = 0;
	static const char mask[] = {0, 63, 31, 15, 7, 3, 1, 0};

	do {
		if (!file.getChar(&c))
			return false;
	} while ((uchar)c >= 0x80);

	while (true) {
		if (!file.getChar(&c))
			return false;
		if (c == '\0')
			break;

		pixel = (c & 0x7f) >> (7 - bits);
		multiplier = c & mask[(int)bits];

		while ((uchar)c >= 0x80) {
			if (!file.getChar(&c))
				return false;
			multiplier = (multiplier << 7) + (c & 0x7f);
		}
		multiplier++;
		if (written + multiplier > _size.width())
			multiplier = _size.width() - written;
		memset(buf + written, pixel - 1, multiplier);
		written += multiplier;
	}

	while (written < _size.width())
		buf[written++] = pixel - 1;

	return true;
}

QImage BSBMap::readImage()
{
	QFile file(path());
	char bits;

	if (!file.open(QIODevice::ReadOnly))
		return QImage();
	if (!(file.seek(_dataOffset) && file.getChar(&bits)))
		return QImage();

	QImage img(_size, QImage::Format_Indexed8);
	img.setColorTable(_palette);

	for (int row = 0; row < _size.height(); row++) {
		uchar *bsb_row = img.scanLine(row);
		if (!readRow(file, bits, bsb_row))
			return QImage();
	}

	return img;
}

BSBMap::BSBMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _img(0), _ratio(1.0), _dataOffset(-1), _valid(false)
{
	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly)) {
		_errorString = fileName + ": " + file.errorString();
		return;
	}

	_palette.resize(256);
	if (!readHeader(file))
		return;
	_dataOffset = file.pos();

	_valid = true;
}

BSBMap::~BSBMap()
{
	delete _img;
}

QPointF BSBMap::ll2xy(const Coordinates &c)
{
	return QPointF(_transform.proj2img(_projection.ll2xy(c))) / _ratio;
}

Coordinates BSBMap::xy2ll(const QPointF &p)
{
	return _projection.xy2ll(_transform.img2proj(p * _ratio));
}

QRectF BSBMap::bounds()
{
	return _skewSize.isValid()
	  ? QRectF(QPointF(0, 0), _skewSize / _ratio)
	  : QRectF(QPointF(0, 0), _size / _ratio);
}

void BSBMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	if (_img)
		_img->draw(painter, rect, flags);
}

void BSBMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(deviceRatio);

	_ratio = mapRatio;
	if (_img)
		_img->setDevicePixelRatio(_ratio);
}

void BSBMap::load()
{
	if (!_img) {
		if (_skew > 0.0 && _skew < 360.0) {
			QTransform matrix;
			matrix.rotate(-_skew);
			_img = new Image(readImage().transformed(matrix));
		} else
			_img = new Image(readImage());
	}
}

void BSBMap::unload()
{
	delete _img;
	_img = 0;
}
