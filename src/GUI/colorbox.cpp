#include <QStylePainter>
#include <QStyleOptionButton>
#include <QMouseEvent>
#include <QColorDialog>
#include <QComboBox>
#include "colorbox.h"


ColorBox::ColorBox(QWidget *parent) : QWidget(parent)
{
	_color = Qt::red;
	_alpha = true;
	_pressed = false;

	setSizePolicy(QSizePolicy::QSizePolicy::Minimum, QSizePolicy::Fixed);
	setAttribute(Qt::WA_Hover);
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

	QStyleOptionButton option;
	option.initFrom(this);
	option.palette.setColor(QPalette::Button, _color);
	if (_pressed)
		option.state |= QStyle::State_Sunken;
	else
		option.state |= QStyle::State_Raised;

#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	static const QSet<QString> set =
	  {"breeze", "windows", "windowsvista"};

	if (set.contains(painter.style()->name().toLower()))
#endif // QT6
		painter.drawPrimitive(QStyle::PE_PanelButtonCommand, option);
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	else
		painter.drawPrimitive(QStyle::PE_PanelButtonBevel, option);
#endif // QT6
}

void ColorBox::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		_pressed = true;
		update();
	}
}

void ColorBox::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		_pressed = false;
		update();

		QColorDialog::ColorDialogOptions options = _alpha
		  ? QColorDialog::ColorDialogOptions(QColorDialog::ShowAlphaChannel)
		  : QColorDialog::ColorDialogOptions();
		QColor color = QColorDialog::getColor(_color, this, QString(), options);
		if (color.isValid()) {
			_color = color;
			update();
			emit colorChanged(_color);
		}
	}

	QWidget::mouseReleaseEvent(event);
}

void ColorBox::setColor(const QColor &color)
{
	_color = color;
	update();
}
