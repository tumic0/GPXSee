#include <QPainter>
#include <QImage>
#include <QtMath>
#include "bitmapline.h"


static QImage img2line(const QImage &img, int width, int offset)
{
	Q_ASSERT(img.format() == QImage::Format_ARGB32_Premultiplied);
	QImage res(width, img.height(), QImage::Format_ARGB32_Premultiplied);
	const int srcBpl = img.bytesPerLine();
	const int dstBpl = res.bytesPerLine();
	const uchar *srcBits = img.bits();
	uchar *dstBits = res.bits();

	for (int i = 0; i < img.height(); i++) {
		const uchar *srcLine = srcBits + srcBpl * i;
		uchar *dstLine = dstBits + dstBpl * i;
		int size = 0;

		if (offset) {
			size = qMin(dstBpl, srcBpl - 4 * offset);
			memcpy(dstLine, srcLine + 4 * offset, size);
			dstLine += size;
		}
		for (int j = dstBpl - size; j > 0; j -= srcBpl, dstLine += srcBpl)
			memcpy(dstLine, srcLine, qMin(j, srcBpl));
	}

	res.setDevicePixelRatio(img.devicePixelRatio());

	return res;
}

void BitmapLine::draw(QPainter *painter, const QPolygonF &line,
  const QImage &img)
{
	int offset = 0;

	for (int i = 1; i < line.size(); i++) {
		QLineF segment(line.at(i-1).x(), line.at(i-1).y(), line.at(i).x(),
		  line.at(i).y());
		int len = qCeil(segment.length() * img.devicePixelRatio());

		painter->save();
		painter->translate(segment.p1());
		painter->rotate(-segment.angle());
		painter->drawImage(0.0, -img.height()/2.0, img2line(img, len, offset));
		painter->restore();

		offset = (len + offset) % img.width();
	}
}

void BitmapLine::draw(QPainter *painter, const QVector<QPolygonF> &lines,
  const QImage &img)
{
	for (int i = 0; i < lines.size(); i++)
		draw(painter, lines.at(i), img);
}
