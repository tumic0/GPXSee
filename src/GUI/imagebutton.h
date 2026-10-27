#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QAbstractButton>

class ImageButton : public QAbstractButton
{
	Q_OBJECT

public:
	ImageButton(const QPixmap &normal, const QPixmap &active, int size,
	  QWidget *parent = 0);

	QSize sizeHint() const {return iconSize();}

protected:
	void paintEvent(QPaintEvent *event);
};

#endif // IMAGEBUTTON_H
