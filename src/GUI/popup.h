#ifndef POPUP_H
#define POPUP_H

#include <QVector>

class QPoint;
class QString;
class QWidget;
class ImageInfo;

class Popup
{
public:
	static void show(const QPoint &pos, const QVector<ImageInfo> &images, const QString &text, QWidget *w);
	static void clear();
};

#endif // POPUP_H
