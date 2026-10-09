#include <QDesktopServices>
#include <QMouseEvent>
#include <QFileInfo>
#include "thumbnail.h"

QSize Thumbnail::size(const QSize &size, int limit, qreal deviceRatio)
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

	return QSize(width * deviceRatio, height * deviceRatio);
}

Thumbnail::Thumbnail(const QString &path, QWidget *parent)
  : QLabel(parent)
{
	setCursor(Qt::PointingHandCursor);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

#ifdef Q_OS_ANDROID
	_path = path;
#else //Q_OS_ANDROID
	_path = QFileInfo(path).absoluteFilePath();
#endif // Q_OS_ANDROID
}

void Thumbnail::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
#ifdef Q_OS_ANDROID
		QDesktopServices::openUrl(_path);
#else // Q_OS_ANDROID
		QDesktopServices::openUrl(QUrl::fromLocalFile(_path));
#endif // Q_OS_ANDROID

	QLabel::mousePressEvent(event);
}
