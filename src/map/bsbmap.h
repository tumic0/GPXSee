#ifndef BSBMAP_H
#define BSBMAP_H

#include <QColor>
#include "transform.h"
#include "projection.h"
#include "map.h"

class QFile;
class Image;

class BSBMap : public Map
{
	Q_OBJECT

public:
	BSBMap(const QString &fileName, QObject *parent = 0);
	~BSBMap();

	QString name() const {return _name;}

	QRectF bounds();
	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load();
	void unload();
	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	bool parseBSB(const QByteArray &line);
	bool parseKNP(const QByteArray &line, QString &datum, QString &proj,
	  double &pp);
	bool parseKNQ(const QByteArray &line, double params[9]);
	bool parseREF(const QByteArray &line, const QString &datum,
	  const QString &proj, double params[9], QList<ReferencePoint> &points);
	bool parseRGB(const QByteArray &line);
	bool readHeader(QFile &file);
	bool createProjection(const QString &datum, const QString &proj,
	  double params[9], const Coordinates &c);
	bool createTransform(QList<ReferencePoint> &points);
	QImage readImage();
	bool readRow(QFile &file, char bits, uchar *buf);

	QString _name;
	Projection _projection;
	Transform _transform;
	qreal _skew;
	Image *_img;
	QSize _size;
	QSize _skewSize;
	qreal _ratio;
	qint64 _dataOffset;
	QVector<QRgb> _palette;

	bool _valid;
	QString _errorString;
};

#endif // BSBMAP_H
