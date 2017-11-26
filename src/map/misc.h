#include <QRectF>

inline QRectF scaled(const QRectF &rect, qreal factor)
{
	return QRectF(QPointF(rect.left() * factor, rect.top() * factor),
	  QSizeF(rect.width() * factor, rect.height() * factor));
}
