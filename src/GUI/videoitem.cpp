#include <QUrl>
#include "videoitem.h"

VideoItem::VideoItem(const QString &file, QGraphicsItem *parent)
  : QGraphicsVideoItem(parent)
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

void VideoItem::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::LoadedMedia
	  || status == QMediaPlayer::BufferedMedia)
		emit videoLoaded();
}
