#ifndef IMAGETHUMBNAIL_H
#define IMAGETHUMBNAIL_H

#include <QLabel>

class ImageThumbnail : public QLabel
{
public:
	ImageThumbnail(const QString &path, int limit, QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent *event);

private:
	QUrl _url;
};

#endif // IMAGETHUMBNAIL_H
