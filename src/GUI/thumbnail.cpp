#include <QImageReader>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QVideoFrame>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QVideoProbe>
#else // QT 6
#include <QVideoSink>
#endif // QT 6.0
#include "thumbnail.h"

static QSize thumbnailSize(const QSize &size, int limit, qreal deviceRatio)
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

Thumbnail::Thumbnail(const QString &path, int limit, bool video, QWidget *parent)
  : QLabel(parent)
{
	if (video) {
		_limit = limit;
		_player = new QMediaPlayer(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		_probe = new QVideoProbe(this);
		_probe->setSource(_player);
		connect(_probe, &QVideoProbe::videoFrameProbed, this,
		  &Thumbnail::capture);
		_player->setMedia(QUrl::fromLocalFile(path));
#else // QT 6
		_player->setAudioOutput(0);
		_sink = new QVideoSink(this);
		_player->setVideoSink(_sink);
		connect(_sink, &QVideoSink::videoFrameChanged, this,
		  &Thumbnail::capture);
		_player->setSource(QUrl::fromLocalFile(path));
#endif // QT 6
		_player->setPosition(0);
		_player->play();
		_player->pause();
	} else {
		qreal r = devicePixelRatioF();
		QImageReader reader(path);
		reader.setAutoTransform(true);
		reader.setScaledSize(thumbnailSize(reader.size(), limit, r));
		QPixmap pm(QPixmap::fromImage(reader.read()));
		pm.setDevicePixelRatio(r);
		setPixmap(pm);
	}

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

void Thumbnail::capture(const QVideoFrame &frame)
{
	if (frame.isValid()) {
		qreal r = devicePixelRatioF();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QImage img(frame.image());
#else // QT 6
		QImage img(frame.toImage());
#endif // QT 6
		QPixmap pm(QPixmap::fromImage(img.scaled(thumbnailSize(frame.size(),
		  _limit, r), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
		pm.setDevicePixelRatio(r);
		setPixmap(pm);
	}
}
