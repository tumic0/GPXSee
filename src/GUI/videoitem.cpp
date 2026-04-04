#include <QUrl>
#include "videoitem.h"

VideoItem::VideoItem(const QString &file, QGraphicsItem *parent)
  : QGraphicsVideoItem(parent), _pos(0), _eos(0)
{
	_player = new QMediaPlayer(this);
	_player->setVideoOutput(this);
	connect(_player, &QMediaPlayer::mediaStatusChanged, this,
	  &VideoItem::mediaStatusChanged);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	_player->setMedia(QUrl::fromLocalFile(file));
#else
	_player->setSource(QUrl::fromLocalFile(file));
#endif
	_player->pause();
}

void VideoItem::seek()
{
	qreal f = _eos / (_duration * 1000.0);
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
		_eos = qMax(0ll, _player->duration() - 100);
		seek();
	}
}
