#ifndef INVALIDMAP_H
#define INVALIDMAP_H

#include "map.h"

class InvalidMap : public Map
{
	Q_OBJECT

public:
	InvalidMap(const QString &fileName, const QString &error, QObject *parent = 0)
	  : Map(fileName, parent), _errorString(error) {}

	QString name() const {return QString();}

	QRectF bounds() {return QRectF();}

	QPointF ll2xy(const Coordinates &) {return QPointF();}
	Coordinates xy2ll(const QPointF &) {return Coordinates();}

	void draw(QPainter *, const QRectF &, Flags) {}

	bool isValid() const {return false;}
	bool isReady() const {return false;}
	QString errorString() const {return _errorString;}

private:
	QString _errorString;
};

#endif // INVALIDMAP_H
