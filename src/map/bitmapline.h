#ifndef BITMAPLINE_H
#define BITMAPLINE_H

#include <QVector>

class QPainter;
class QImage;
class QPolygonF;

namespace BitmapLine
{
	void draw(QPainter *painter, const QPolygonF &line, const QImage &img);
	void draw(QPainter *painter, const QVector<QPolygonF> &lines,
	  const QImage &img);
}

#endif // BITMAPLINE_H
