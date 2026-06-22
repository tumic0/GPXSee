#include <QStylePainter>
#include <QStyleOptionButton>
#include <QColorDialog>
#include <QComboBox>
#include "colorbox.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
static bool useButtonCommand(const QStyle *style)
{
	static const QStringList list = {"breeze", "oxygen", "windowsvista"};
	return list.contains(style->name().toLower());
}
#else // QT6
#define useButtonCommand(style) true
#endif // QT6

ColorBox::ColorBox(QWidget *parent) : QAbstractButton(parent)
{
	_color = Qt::red;
	_alpha = true;

	setSizePolicy(QSizePolicy::QSizePolicy::Minimum, QSizePolicy::Fixed);
	setAttribute(Qt::WA_Hover);

	connect(this, &ColorBox::clicked, this, &ColorBox::showColorDialog);
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
	if (isDown())
		option.state |= QStyle::State_Sunken;
	else
		option.state |= QStyle::State_Raised;

	if (useButtonCommand(painter.style()))
		painter.drawPrimitive(QStyle::PE_PanelButtonCommand, option);
	else
		painter.drawPrimitive(QStyle::PE_PanelButtonBevel, option);
}

void ColorBox::setColor(const QColor &color)
{
	_color = color;
	update();
}

void ColorBox::showColorDialog()
{
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
