#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QStyleOptionButton>
#include <QMouseEvent>
#include <QColorDialog>
#include <QComboBox>
#include "colorbox.h"


ColorBox::ColorBox(QWidget *parent) : QWidget(parent)
{
	_color = Qt::red;
	_alpha = true;

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	static const QSet<QString> set =
	  {"breeze", "fusion", "windows", "windowsvista"};

	if (set.contains(painter.style()->name().toLower())) {
#endif // QT6
		QStyleOptionFrame option;
		option.initFrom(this);
		option.palette.setColor(QPalette::Base, _color);
		option.lineWidth = 1;
		option.midLineWidth = 0;
		option.state |= QStyle::State_Sunken;
		option.features = QStyleOptionFrame::None;

		painter.drawPrimitive(QStyle::PE_PanelLineEdit, option);
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	} else {
		QStyleOptionButton option;
		option.initFrom(this);
		option.palette.setColor(QPalette::Button, _color);

		painter.drawPrimitive(QStyle::PE_PanelButtonBevel, option);
	}
#endif // QT6
}

void ColorBox::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
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
