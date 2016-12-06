#include <QPen>
#include <QPainter>
#include <QResizeEvent>
#include "stylecombobox.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

StyleComboBox::StyleComboBox(QWidget *parent) : QComboBox(parent)
{
	Qt::PenStyle styles[] = {Qt::SolidLine, Qt::DashLine, Qt::DotLine,
	  Qt::DashDotLine, Qt::DashDotDotLine};

	QSize is = iconSize();
	setIconSize(QSize(sizeHint().width(), is.height()));
	is = iconSize();

	for (size_t i = 0; i < ARRAY_SIZE(styles); i++) {
		QPixmap pm(is);
		pm.fill(Qt::transparent);

		QBrush brush(Qt::black);
		QPen pen(brush, is.height() / 7, styles[i]);

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

void StyleComboBox::resizeEvent(QResizeEvent *event)
{
	QSize is = iconSize();
	setIconSize(QSize(event->size().width() - 30, is.height()));
	is = iconSize();

	for (int i = 0; i < count(); i++) {
		QPixmap pm(is);
		pm.fill(Qt::transparent);

		QBrush brush(Qt::black);
		QPen pen(brush, is.height() / 7, (Qt::PenStyle) itemData(i).toInt());

		QPainter painter(&pm);
		painter.setPen(pen);
		painter.drawLine(0, is.height() / 2, is.width(), is.height() / 2);

		setItemIcon(i, QIcon(pm));
	}
}
