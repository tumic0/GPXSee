#include <QPainter>
#include <QImage>
#include "bitmapline.h"

static QImage img2line(const QImage &img, int width)
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

		for (int j = dstBpl; j > 0; j -= srcBpl, dstLine += srcBpl)
			memcpy(dstLine, srcLine, qMin(j, srcBpl));
	}

	return res;
}

void BitmapLine::draw(QPainter *painter, const QPolygonF &line,
  const QImage &img)
{
	for (int i = 1; i < line.size(); i++) {
		QLineF segment(line.at(i-1).x(), line.at(i-1).y(), line.at(i).x(),
		  line.at(i).y());

		painter->save();
		painter->translate(segment.p1());
		painter->rotate(-segment.angle());
		painter->drawImage(0, -img.height()/2, img2line(img, segment.length()));
		painter->restore();
	}
}

