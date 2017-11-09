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

void TrackInfo::plot(QPainter *painter, const QRectF &target, qreal scale)
{
	QSizeF canvas = QSizeF(target.width() / scale, target.height() / scale);
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
