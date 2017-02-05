#include "fliplabel.h"


void FlipLabel::addItem(const QString &key, const QString &value)
{
	_labels.insert(key, value);

	if (_labels.size() == 1)
		_it = _labels.constBegin();

	QLabel::setText(_it.value());
	QLabel::setToolTip(_it.key());
}

void FlipLabel::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);

	if (!_labels.count())
		return;

	_it++;
	if (_it == _labels.constEnd())
		_it = _labels.constBegin();

	QLabel::setText(_it.value());
	QLabel::setToolTip(_it.key());
}
