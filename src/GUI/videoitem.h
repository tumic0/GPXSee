#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QGraphicsVideoItem>
#include <QMediaPlayer>

class VideoItem : public QGraphicsVideoItem
{
	Q_OBJECT

public:
	VideoItem(const QString &file, QGraphicsItem *parent = 0);

	void seek(qint64 pos);

private slots:
	void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
	QMediaPlayer *_player;
	qint64 _pos, _eos;
};

#endif // VIDEOITEM_H
