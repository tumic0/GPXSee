#include <QStylePainter>
#include <QStyleOptionComboBox>
#include <QMouseEvent>
#include <QColorDialog>
#include <QComboBox>
#include "colorbox.h"


ColorBox::ColorBox(QWidget *parent) : QWidget(parent)
{
	_color = Qt::red;
	setSizePolicy(QSizePolicy::QSizePolicy::Minimum, QSizePolicy::Fixed);
}

QSize ColorBox::sizeHint() const
{
	static QSize size;
	if (size.isValid())
		return size;

	QComboBox cb;
	size = cb.sizeHint();
	return size;
}

void ColorBox::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

    QStylePainter painter(this);

    QStyleOptionComboBox option;
    option.initFrom(this);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
	painter.setBrush(_color);
	painter.drawPrimitive(QStyle::PE_Frame, option);
#else // Q_OS_MAC || Q_OS_WIN32
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	painter.setBrush(_color);
	painter.drawRect(event->rect().adjusted(-1, -1, 0, 0));
	painter.drawPrimitive(QStyle::PE_FrameLineEdit, option);
#else // QT 5
	option.palette.setBrush(QPalette::Base, _color);
	painter.drawPrimitive(QStyle::PE_FrameLineEdit, option);
#endif // QT 5
#endif // Q_OS_MAC || Q_OS_WIN32
}

void ColorBox::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		return;

	QColor color = QColorDialog::getColor(_color, this, QString(),
	  QColorDialog::ShowAlphaChannel);
	if (color.isValid()) {
		_color = color;
		update();
		emit colorChanged(_color);
	}
}

void ColorBox::setColor(const QColor &color)
{
	_color = color;
	update();
}
