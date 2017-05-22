#include "oddspinbox.h"

OddSpinBox::OddSpinBox(QWidget *parent) : QSpinBox(parent)
{
	setSingleStep(2);
	setMinimum(1);
}

QValidator::State OddSpinBox::validate(QString &text, int &pos) const
{
	Q_UNUSED(pos);
	bool ok;
	int val;

	val = text.toInt(&ok);
	if (!ok || val < 0 || val % 2 == 0)
		return QValidator::Invalid;

	return QValidator::Acceptable;
}
