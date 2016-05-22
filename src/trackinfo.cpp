#include <QPaintEngine>
#include <QPaintDevice>
#include "config.h"
#include "infoitem.h"
#include "trackinfo.h"


TrackInfo::TrackInfo(QObject *parent) : QGraphicsScene(parent)
{
	_info = new InfoItem();
	addItem(_info);
}

void TrackInfo::insert(const QString &key, const QString &value)
{
	_info->insert(key, value);
}

void TrackInfo::plot(QPainter *painter, const QRectF &target)
{
	qreal ratio = painter->paintEngine()->paintDevice()->logicalDpiX()
	  / SCREEN_DPI;
	QSizeF canvas = QSizeF(target.width() / ratio, target.height() / ratio);
	QSizeF diff = QSizeF(qAbs(canvas.width() - sceneRect().width()),
	  qAbs(canvas.height() - sceneRect().height()));
	QRectF adj = sceneRect().adjusted(0, -diff.height()/2, diff.width(),
	  diff.height()/2);

	render(painter, target, adj);
}

bool TrackInfo::isEmpty() const
{
	return _info->isEmpty();
}

QSizeF TrackInfo::contentSize() const
{
	return sceneRect().size();
}
