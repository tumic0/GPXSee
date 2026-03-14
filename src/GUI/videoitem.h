#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QGraphicsVideoItem>
#include <QMediaPlayer>

class VideoItem : public QGraphicsVideoItem
{
	Q_OBJECT

public:
	VideoItem(const QString &file, QGraphicsItem *parent = 0);

	void seek(qint64 pos) {_player->setPosition(pos);}

signals:
	void videoLoaded();

private slots:
	void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
	QMediaPlayer *_player;
};

#endif // VIDEOITEM_H
