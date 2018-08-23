#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include "map.h"

class QPainter;

class Image
{
public:
	Image(const QString &fileName);

	QSize size() const {return _img.size();}
	void draw(QPainter *painter, const QRectF &rect, Map::Flags flags);
	void setDevicePixelRatio(qreal ratio);

private:
	QImage _img;
	qreal _ratio;
};

#endif // IMAGE_H
