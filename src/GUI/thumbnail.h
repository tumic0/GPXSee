#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QLabel>

class Thumbnail : public QLabel
{
public:
	Thumbnail(const QString &path, QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent *event);
	static QSize size(const QSize &size, int limit, qreal deviceRatio);

private:
	QString _path;
};

#endif // THUMBNAIL_H
