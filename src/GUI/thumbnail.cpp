#include <QImageReader>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMouseEvent>
#include "thumbnail.h"

static QSize thumbnailSize(const QSize &size, int limit)
{
	int width, height;
	if (size.width() > size.height()) {
		width = qMin(size.width(), limit);
		qreal ratio = size.width() / (qreal)size.height();
		height = (int)(width / ratio);
	} else {
		height = qMin(size.height(), limit);
		qreal ratio = size.height() / (qreal)size.width();
		width = (int)(height / ratio);
	}

	return QSize(width, height);
}

Thumbnail::Thumbnail(const QString &path, int limit, QWidget *parent)
  : QLabel(parent)
{
	QImageReader reader(path);
	reader.setAutoTransform(true);
	reader.setScaledSize(thumbnailSize(reader.size(), limit));
	setPixmap(QPixmap::fromImage(reader.read()));

	setCursor(Qt::PointingHandCursor);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	_path = QFileInfo(path).absoluteFilePath();
}

void Thumbnail::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		QDesktopServices::openUrl(QUrl::fromLocalFile(_path));
}
