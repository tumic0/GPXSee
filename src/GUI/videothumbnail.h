#ifndef VIDEOTHUMBNAIL_H
#define VIDEOTHUMBNAIL_H

#include <QVideoWidget>

class VideoThumbnail : public QVideoWidget
{
public:
	VideoThumbnail(const QString &path, int limit, QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent *event);

private:
	QString _path;
};

#endif // VIDEOTHUMBNAIL_H
