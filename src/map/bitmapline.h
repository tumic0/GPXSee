#ifndef BITMAPLINE_H
#define BITMAPLINE_H

class QPainter;
class QImage;
class QPolygonF;

namespace BitmapLine
{
	void draw(QPainter *painter, const QPolygonF &line, const QImage &img);
}

#endif // BITMAPLINE_H
