#ifndef VIDEOTHUMBNAIL_H
#define VIDEOTHUMBNAIL_H

#include "thumbnail.h"

class QVideoFrame;
class QMediaPlayer;
class QVideoSink;
class QVideoProbe;

class VideoThumbnail : public Thumbnail
{
	Q_OBJECT

public:
	VideoThumbnail(const QString &path, int limit, QWidget *parent = 0);

private slots:
	void capture(const QVideoFrame &frame);

private:
	QString _path;
	QMediaPlayer *_player;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QVideoProbe *_probe;
#else
	QVideoSink *_sink;
#endif // QT 6.0
	int _limit;
};

#endif // VIDEOTHUMBNAIL_H
