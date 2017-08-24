#ifndef PERCENTSLIDER_H
#define PERCENTSLIDER_H

#include <QWidget>

class QSlider;
class QLabel;

class PercentSlider : public QWidget
{
	Q_OBJECT

public:
	PercentSlider(QWidget *parent = 0);

	int value() const;

public slots:
	void setValue(int value);

private slots:
	void updateLabel(int value);

private:
	QSlider *_slider;
	QLabel *_label;
};

#endif // PERCENTSLIDER_H
