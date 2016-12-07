#include <QPen>
#include <QPainter>
#include <QResizeEvent>
#include "stylecombobox.h"


#define MIN_LINE_LENGTH  60
#define LINE_WIDTH_RATIO 7

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

StyleComboBox::StyleComboBox(QWidget *parent) : QComboBox(parent)
{
	Qt::PenStyle styles[] = {Qt::SolidLine, Qt::DashLine, Qt::DotLine,
	  Qt::DashDotLine, Qt::DashDotDotLine};

	QSize is = iconSize();
	setIconSize(QSize(MIN_LINE_LENGTH, is.height()));
	is = iconSize();

	for (size_t i = 0; i < ARRAY_SIZE(styles); i++) {
		QPixmap pm(is);
		pm.fill(Qt::transparent);

		QBrush brush(Qt::black);
		QPen pen(brush, is.height() / LINE_WIDTH_RATIO, styles[i]);

		QPainter painter(&pm);
		painter.setPen(pen);
		painter.drawLine(0, is.height() / 2, is.width(), is.height() / 2);

		addItem(QIcon(pm), QString(), QVariant((int)styles[i]));
	}
}

void StyleComboBox::setValue(Qt::PenStyle value)
{
	for (int i = 0; i < count(); i++) {
		if ((Qt::PenStyle) itemData(i).toInt() == value) {
			setCurrentIndex(i);
			return;
		}
	}
}
