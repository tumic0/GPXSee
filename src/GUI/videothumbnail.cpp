#include <QDesktopServices>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QVideoFrame>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QVideoProbe>
#else // QT 6
#include <QVideoSink>
#include <QAudioOutput>
#endif // QT 6.0
#include "videothumbnail.h"

VideoThumbnail::VideoThumbnail(const QString &path, int limit, QWidget *parent)
  : Thumbnail(path, parent), _limit(limit)
{
	QMediaPlayer *player = new QMediaPlayer(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QVideoProbe *probe = new QVideoProbe(player);
	probe->setSource(player);
	connect(probe, &QVideoProbe::videoFrameProbed, this,
	  &VideoThumbnail::capture);
	player->setMedia(QUrl::fromLocalFile(path));
#else // QT 6
	QVideoSink *sink = new QVideoSink(player);
	player->setVideoSink(sink);
	player->setAudioOutput(new QAudioOutput(player));
	connect(sink, &QVideoSink::videoFrameChanged, this,
	  &VideoThumbnail::capture);
	player->setSource(QUrl::fromLocalFile(path));
#endif // QT 6
	player->play();
}

void VideoThumbnail::capture(const QVideoFrame &frame)
{
	if (!frame.isValid())
		return;

	qreal r = devicePixelRatioF();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QImage img(frame.image());
#else // QT 6
	QImage img(frame.toImage());
#endif // QT 6
	QPixmap pm(QPixmap::fromImage(img.scaled(size(frame.size(), _limit, r),
	  Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
	pm.setDevicePixelRatio(r);

	setPixmap(pm);
}
