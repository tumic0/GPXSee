#include <QStylePainter>
#include <QStyleOptionComboBox>
#include <QMouseEvent>
#include <QColorDialog>
#include <QComboBox>
#include "colorbox.h"


ColorBox::ColorBox(QWidget *parent) : QWidget(parent)
{
	_color = Qt::red;
	_alpha = true;

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
	// Fallback for some broken QT4 styles that do not draw the background
	painter.setBrush(_color);
	painter.setPen(Qt::NoPen);
	painter.drawRect(event->rect().adjusted(2, 2, -2, -2));
	// If works (QT5 and most QT4 styles) overpaints the previous rectangle
	option.palette.setBrush(QPalette::Base, _color);
	painter.drawPrimitive(QStyle::PE_PanelLineEdit, option);
	painter.drawPrimitive(QStyle::PE_FrameLineEdit, option);
#endif // Q_OS_MAC || Q_OS_WIN32
}

void ColorBox::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		return;
	QColorDialog::ColorDialogOptions options = _alpha
	  ? QColorDialog::ShowAlphaChannel : (QColorDialog::ColorDialogOptions)0;
	QColor color = QColorDialog::getColor(_color, this, QString(), options);
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
