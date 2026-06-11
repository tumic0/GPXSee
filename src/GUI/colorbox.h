#ifndef COLORBOX_H
#define COLORBOX_H

#include <QAbstractButton>

class ColorBox : public QAbstractButton
{
	Q_OBJECT

public:
	ColorBox(QWidget *parent = 0);

	const QColor &color() const {return _color;}
	void setColor(const QColor &color);
	void enableAlphaChannel(bool enable) {_alpha = enable;}

	QSize sizeHint() const;

signals:
	void colorChanged(const QColor &color);

protected:
	void paintEvent(QPaintEvent *event);

private slots:
	void showColorDialog();

private:
	QColor _color;
	bool _alpha;
};

#endif // COLORBOX_H
