#include <QDesktopServices>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QFileInfo>
#include <QAudioOutput>
#include "videothumbnail.h"

VideoThumbnail::VideoThumbnail(const QString &path, int limit, QWidget *parent)
  : QVideoWidget(parent)
{
#ifdef Q_OS_ANDROID
	_url = QUrl(path);
#else // Q_OS_ANDROID
	_url = QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath());
#endif // Q_OS_ANDROID

	setCursor(Qt::PointingHandCursor);
	setMinimumWidth(limit);
	setMinimumHeight(limit * 0.5625);
	setAspectRatioMode(Qt::KeepAspectRatio);

	QMediaPlayer *player = new QMediaPlayer(this);
	player->setVideoOutput(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	player->setMedia(_url);
#else // QT 6
	player->setAudioOutput(new QAudioOutput(this));
	player->setSource(_url);
#endif // QT 6
	player->play();
}

void VideoThumbnail::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		QDesktopServices::openUrl(_url);

	QVideoWidget::mousePressEvent(event);
}
