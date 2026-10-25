#include <QPainter>
#include "imagebutton.h"

ImageButton::ImageButton(const QPixmap &normal, const QPixmap &active,
  QWidget *parent) : QAbstractButton(parent)
{
	QIcon icon;
	icon.addPixmap(normal, QIcon::Normal);
	icon.addPixmap(active, QIcon::Active);

	setIcon(icon);
	setIconSize(QSize(32, 32));
}

void ImageButton::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter p(this);
	icon().paint(&p, rect(), Qt::AlignCenter,
	  isDown() ? QIcon::Active : QIcon::Normal);
}
