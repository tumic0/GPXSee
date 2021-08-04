#include <QImageReader>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMouseEvent>
#include "data/imageinfo.h"
#include "thumbnail.h"

static QSize thumbnailSize(const ImageInfo &img, int limit)
{
	int width, height;
	if (img.size().width() > img.size().height()) {
		width = qMin(img.size().width(), limit);
		qreal ratio = img.size().width() / (qreal)img.size().height();
		height = (int)(width / ratio);
	} else {
		height = qMin(img.size().height(), limit);
		qreal ratio = img.size().height() / (qreal)img.size().width();
		width = (int)(height / ratio);
	}

	return QSize(width, height);
}

Thumbnail::Thumbnail(const ImageInfo &img, int size, QWidget *parent)
  : QLabel(parent)
{
	QImageReader reader(img.path());
	reader.setAutoTransform(true);
	reader.setScaledSize(thumbnailSize(img, size));
	setPixmap(QPixmap::fromImage(reader.read()));

	setCursor(Qt::PointingHandCursor);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	_path = QFileInfo(img.path()).absoluteFilePath();
}

void Thumbnail::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		QDesktopServices::openUrl(QUrl::fromLocalFile(_path));
}
