#include <QSpinBox>
#include <QGridLayout>
#include "units.h"
#include "marginswidget.h"

MarginsWidget::MarginsWidget(QWidget *parent) : QWidget(parent)
{
	_top = new QSpinBox();
	_bottom = new QSpinBox();
	_left = new QSpinBox();
	_right = new QSpinBox();

	_top->setMaximumWidth(_top->sizeHint().width());
	_bottom->setMaximumWidth(_bottom->sizeHint().width());
	_left->setMaximumWidth(_left->sizeHint().width());
	_right->setMaximumWidth(_right->sizeHint().width());

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(_top, 0, 0, 1, 2, Qt::AlignCenter);
	layout->addWidget(_left, 1, 0, 1, 1, Qt::AlignRight);
	layout->addWidget(_right, 1, 1, 1, 1, Qt::AlignLeft);
	layout->addWidget(_bottom, 2, 0, 1, 2, Qt::AlignCenter);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setLayout(layout);
}

void MarginsWidget::setValue(const QMargins &value)
{
	_top->setValue(value.top());
	_bottom->setValue(value.bottom());
	_left->setValue(value.left());
	_right->setValue(value.right());
}

void MarginsWidget::setUnits(const QString &units)
{
	_top->setSuffix(UNIT_SPACE + units);
	_bottom->setSuffix(UNIT_SPACE + units);
	_left->setSuffix(UNIT_SPACE + units);
	_right->setSuffix(UNIT_SPACE + units);

	_top->setMaximumWidth(_top->sizeHint().width());
	_bottom->setMaximumWidth(_bottom->sizeHint().width());
	_left->setMaximumWidth(_left->sizeHint().width());
	_right->setMaximumWidth(_right->sizeHint().width());
}

QMargins MarginsWidget::value() const
{
	return QMargins(_left->value(), _top->value(), _right->value(),
	  _bottom->value());
}


MarginsFWidget::MarginsFWidget(QWidget *parent) : QWidget(parent)
{
	_top = new QDoubleSpinBox();
	_bottom = new QDoubleSpinBox();
	_left = new QDoubleSpinBox();
	_right = new QDoubleSpinBox();

	_top->setMaximumWidth(_top->sizeHint().width());
	_bottom->setMaximumWidth(_bottom->sizeHint().width());
	_left->setMaximumWidth(_left->sizeHint().width());
	_right->setMaximumWidth(_right->sizeHint().width());

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(_top, 0, 0, 1, 2, Qt::AlignCenter);
	layout->addWidget(_left, 1, 0, 1, 1, Qt::AlignRight);
	layout->addWidget(_right, 1, 1, 1, 1, Qt::AlignLeft);
	layout->addWidget(_bottom, 2, 0, 1, 2, Qt::AlignCenter);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setLayout(layout);
}

void MarginsFWidget::setValue(const MarginsF &value)
{
	_top->setValue(value.top());
	_bottom->setValue(value.bottom());
	_left->setValue(value.left());
	_right->setValue(value.right());
}

void MarginsFWidget::setUnits(const QString &units)
{
	_top->setSuffix(UNIT_SPACE + units);
	_bottom->setSuffix(UNIT_SPACE + units);
	_left->setSuffix(UNIT_SPACE + units);
	_right->setSuffix(UNIT_SPACE + units);

	_top->setMaximumWidth(_top->sizeHint().width());
	_bottom->setMaximumWidth(_bottom->sizeHint().width());
	_left->setMaximumWidth(_left->sizeHint().width());
	_right->setMaximumWidth(_right->sizeHint().width());
}

void MarginsFWidget::setSingleStep(qreal step)
{
	_top->setSingleStep(step);
	_bottom->setSingleStep(step);
	_left->setSingleStep(step);
	_right->setSingleStep(step);
}

MarginsF MarginsFWidget::value() const
{
	return MarginsF(_left->value(), _top->value(), _right->value(),
	  _bottom->value());
}
