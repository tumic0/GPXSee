#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include "map.h"

class QPainter;

class Image
{
public:
	Image(const QString &fileName) : _img(fileName) {}
	Image(const QImage &img) : _img(img) {}

	void draw(QPainter *painter, const QRectF &rect, Map::Flags flags);
	void setDevicePixelRatio(qreal ratio);

private:
	QImage _img;
};

#endif // IMAGE_H
