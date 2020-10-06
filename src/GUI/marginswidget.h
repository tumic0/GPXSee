#ifndef MARGINSWIDGET_H
#define MARGINSWIDGET_H

#include <QWidget>
#include <QMargins>
#include "margins.h"

class QSpinBox;
class QDoubleSpinBox;

class MarginsWidget : public QWidget
{
	Q_OBJECT

public:
	MarginsWidget(QWidget *parent = 0);

	QMargins value() const;
	void setValue(const QMargins &value);
	void setUnits(const QString &units);

private:
	QSpinBox *_top;
	QSpinBox *_bottom;
	QSpinBox *_left;
	QSpinBox *_right;
};

class MarginsFWidget : public QWidget
{
	Q_OBJECT

public:
	MarginsFWidget(QWidget *parent = 0);

	MarginsF value() const;
	void setValue(const MarginsF &value);
	void setUnits(const QString &units);
	void setSingleStep(qreal step);

private:
	QDoubleSpinBox *_top;
	QDoubleSpinBox *_bottom;
	QDoubleSpinBox *_left;
	QDoubleSpinBox *_right;
};

#endif // MARGINSWIDGET_H
