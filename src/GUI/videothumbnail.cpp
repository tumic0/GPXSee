#include <QDesktopServices>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QFileInfo>
#include <QAudioOutput>
#include "videothumbnail.h"

VideoThumbnail::VideoThumbnail(const QString &path, int limit, QWidget *parent)
  : QVideoWidget(parent), _path(path)
{
#ifdef Q_OS_ANDROID
	_path = path;
	QUrl url(_path);
#else // Q_OS_ANDROID
	_path = QFileInfo(path).absoluteFilePath();
	QUrl url(QUrl::fromLocalFile(_path));
#endif // Q_OS_ANDROID

	setCursor(Qt::PointingHandCursor);
	setMinimumWidth(limit);
	setMinimumHeight(limit * 0.5625);
	setAspectRatioMode(Qt::KeepAspectRatio);

	QMediaPlayer *player = new QMediaPlayer(this);
	player->setVideoOutput(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	player->setMedia(url);
#else // QT 6
	player->setAudioOutput(new QAudioOutput(this));
	player->setSource(url);
#endif // QT 6
	player->play();
}

void VideoThumbnail::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
#ifdef Q_OS_ANDROID
		QDesktopServices::openUrl(_path);
#else // Q_OS_ANDROID
		QDesktopServices::openUrl(QUrl::fromLocalFile(_path));
#endif // Q_OS_ANDROID

	QVideoWidget::mousePressEvent(event);
}