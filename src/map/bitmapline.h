#ifndef BITMAPLINE_H
#define BITMAPLINE_H

#include <QVector>

class QPainter;
class QImage;
class QPolygonF;
class QPainterPath;

namespace BitmapLine
{
	void draw(QPainter *painter, const QPolygonF &line, const QImage &img);
	void draw(QPainter *painter, const QVector<QPolygonF> &lines,
	  const QImage &img);
	void draw(QPainter *painter, const QPainterPath &line, const QImage &img);
}

#endif // BITMAPLINE_H
