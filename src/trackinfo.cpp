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
	render(painter, target);
}

bool TrackInfo::isEmpty()
{
	return _info->isEmpty();
}
