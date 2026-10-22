#include <QUrl>
#include "videoitem.h"

VideoItem::VideoItem(const QString &file, QGraphicsItem *parent)
  : QGraphicsVideoItem(parent), _pos(0), _eos(0)
{
	_player = new QMediaPlayer(this);
	_player->setVideoOutput(this);
	connect(_player, &QMediaPlayer::mediaStatusChanged, this,
	  &VideoItem::mediaStatusChanged);

#ifdef Q_OS_ANDROID
	QUrl url(file);
#else // Q_OS_ANDROID
	QUrl url(QUrl::fromLocalFile(file));
#endif // Q_OS_ANDROID
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	_player->setMedia(url);
#else
	_player->setSource(url);
#endif
	_player->pause();
}

void VideoItem::seek()
{
	qreal f = qMin(1.0, _eos / (_duration * 1000.0));
	_player->setPosition((qint64)(_pos * 1000.0 * f));
}

void VideoItem::seek(qreal pos, qreal duration)
{
	_pos = pos;
	_duration = duration;

	QMediaPlayer::MediaStatus status = _player->mediaStatus();
	if (status == QMediaPlayer::LoadedMedia
	  || status == QMediaPlayer::BufferedMedia)
		seek();
}

void VideoItem::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::LoadedMedia
	  || status == QMediaPlayer::BufferedMedia) {
		/* Seeking to the end of the file makes the video disappear, so only
		   allow seeking to 1/3s before the end */
		_eos = qMax(0ll, _player->duration() - 333);
		seek();
	}
}
