#ifndef QCTMAP_H
#define QCTMAP_H

#include <QFile>
#include <QRgb>
#include "map.h"

class QDataStream;

class QCTMap : public Map
{
public:
	QCTMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, bool *isDir);

private:
	bool readName(QDataStream &stream);
	bool readSize(QDataStream &stream);
	bool readDatumShift(QDataStream &stream);
	bool readHeader(QDataStream &stream);
	bool readGeoRef(QDataStream &stream);
	bool readIndex(QDataStream &stream);
	bool readPalette(QDataStream &stream);
	QPixmap tile(int x, int y);

	QFile _file;
	QString _name;
	int _rows, _cols;
	double _lon, _lonX, _lonXX, _lonXXX, _lonY, _lonYY, _lonYYY, _lonXY,
	  _lonXXY, _lonXYY;
	double _lat, _latX, _latXX, _latXXX, _latY, _latYY, _latYYY, _latXY,
	  _latXXY, _latXYY;
	double _eas, _easY, _easX, _easYY, _easXY, _easXX, _easYYY, _easYYX,
	  _easXXY, _easXXX;
	double _nor, _norY, _norX, _norYY, _norXY, _norXX, _norYYY, _norYYX,
	  _norXXY, _norXXX;
	double _shiftE, _shiftN;
	QVector<quint32> _index;
	QVector<QRgb> _palette;

	qreal _mapRatio;
	bool _valid;
	QString _errorString;
};

#endif // QCTMAP_H
