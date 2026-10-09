#include <QImageReader>
#include <QFileInfo>
#include "imagethumbnail.h"

ImageThumbnail::ImageThumbnail(const QString &path, int limit, QWidget *parent)
  : Thumbnail(path, parent)
{
	qreal r = devicePixelRatioF();
	QImageReader reader(path);
	reader.setAutoTransform(true);
	reader.setScaledSize(size(reader.size(), limit, r));
	QPixmap pm(QPixmap::fromImage(reader.read()));
	pm.setDevicePixelRatio(r);

	setPixmap(pm);
}
