#ifndef ODDSPINBOX_H
#define ODDSPINBOX_H

#include <QSpinBox>

class OddSpinBox : public QSpinBox
{
public:
	OddSpinBox(QWidget *parent = 0);

protected:
	QValidator::State validate(QString &text, int &pos) const;
};

#endif // ODDSPINBOX_H
