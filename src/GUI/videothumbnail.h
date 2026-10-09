#ifndef VIDEOTHUMBNAIL_H
#define VIDEOTHUMBNAIL_H

#include "thumbnail.h"

class QVideoFrame;

class VideoThumbnail : public Thumbnail
{
	Q_OBJECT

public:
	VideoThumbnail(const QString &path, int limit, QWidget *parent = 0);

private slots:
	void capture(const QVideoFrame &frame);

private:
	QString _path;
	int _limit;
};

#endif // VIDEOTHUMBNAIL_H
