#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QLabel>

class ImageInfo;
class QVideoFrame;
class QMediaPlayer;
class QVideoSink;

class Thumbnail : public QLabel
{
	Q_OBJECT

public:
	Thumbnail(const QString &path, int limit, bool video = false,
	  QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent *event);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
private slots:
	void capture(const QVideoFrame &frame);
#endif // QT 6.0

private:
	QString _path;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QMediaPlayer *_player;
	QVideoSink *_sink;
	int _limit;
#endif // QT 6.0
};

#endif // THUMBNAIL_H
