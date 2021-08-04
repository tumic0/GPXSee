#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QLabel>

class ImageInfo;

class Thumbnail : public QLabel
{
public:
	Thumbnail(const ImageInfo &img, int size, QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent *event);

private:
	QString _path;
};

#endif // THUMBNAIL_H
