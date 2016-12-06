#ifndef COLORBOX_H
#define COLORBOX_H

#include <QWidget>

class ColorBox : public QWidget
{
	Q_OBJECT

public:
	ColorBox(QWidget *parent = 0);

	const QColor &color() const {return _color;}
	void setColor(const QColor &color);

	QSize sizeHint() const;

signals:
	void colorChanged(const QColor &color);

protected:
	void mousePressEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

private:
	QColor _color;
};

#endif // COLORBOX_H
