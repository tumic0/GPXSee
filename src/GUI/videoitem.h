#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QGraphicsVideoItem>
#include <QMediaPlayer>

class VideoItem : public QGraphicsVideoItem
{
	Q_OBJECT

public:
	VideoItem(const QString &file, QGraphicsItem *parent = 0);

	void seek(qreal pos, qreal duration);

private slots:
	void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
	void seek();

	QMediaPlayer *_player;
	qreal _pos, _duration;
	qint64 _eos;
};

#endif // VIDEOITEM_H
