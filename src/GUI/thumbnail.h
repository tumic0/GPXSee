#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QLabel>

class ImageInfo;
class QVideoFrame;
class QMediaPlayer;
class QVideoSink;
class QVideoProbe;

class Thumbnail : public QLabel
{
	Q_OBJECT

public:
	Thumbnail(const QString &path, int limit, bool video = false,
	  QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent *event);

private slots:
	void capture(const QVideoFrame &frame);

private:
	QString _path;
	QMediaPlayer *_player;
	int _limit;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QVideoProbe *_probe;
#else
	QVideoSink *_sink;
#endif // QT 6.0
};

#endif // THUMBNAIL_H
