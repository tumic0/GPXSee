#include <QDesktopServices>
#include <QMouseEvent>
#include <QImageReader>
#include <QFileInfo>
#include "imagethumbnail.h"

static QSize size(const QSize &size, int limit, qreal deviceRatio)
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

ImageThumbnail::ImageThumbnail(const QString &path, int limit, QWidget *parent)
  : QLabel(parent)
{
	setCursor(Qt::PointingHandCursor);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

#ifdef Q_OS_ANDROID
	_url = QUrl(path);
#else //Q_OS_ANDROID
	_url = QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath());
#endif // Q_OS_ANDROID

	qreal r = devicePixelRatioF();
	QImageReader reader(path);
	reader.setAutoTransform(true);
	reader.setScaledSize(::size(reader.size(), limit, r));
	QPixmap pm(QPixmap::fromImage(reader.read()));
	pm.setDevicePixelRatio(r);

	setPixmap(pm);
}

void ImageThumbnail::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		QDesktopServices::openUrl(_url);

	QLabel::mousePressEvent(event);
}
