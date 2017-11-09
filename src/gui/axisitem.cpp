#include <cmath>
#include <QPainter>
#include "config.h"
#include "misc.h"
#include "axisitem.h"


#define AXIS_WIDTH  1
#define TICK        6
#define PADDING     6
#define XTICKS      15
#define YTICKS      10

struct Label {
	double min;
	double max;
	double d;
};

static struct Label label(double min, double max, int ticks)
{
	double range;
	struct Label l;

	range = niceNum(max - min, 0);
	l.d = niceNum(range / ticks, 1);
	l.min = ceil(min / l.d) * l.d;
	l.max = floor(max / l.d) * l.d;

	return l;
}


AxisItem::AxisItem(Type type, QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_type = type;
	_size = 0;

#ifndef Q_OS_MAC
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif // Q_OS_MAC
}

void AxisItem::setRange(const RangeF &range)
{
	prepareGeometryChange();
	_range = range;
	updateBoundingRect();
	update();
}

void AxisItem::setSize(qreal size)
{
	prepareGeometryChange();
	_size = size;
	updateBoundingRect();
	update();
}

void AxisItem::setLabel(const QString& label)
{
	prepareGeometryChange();
	_label = label;
	updateBoundingRect();
	update();
}

void AxisItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ss, es, ls;
	struct Label l;


	l = label(_range.min(), _range.max(), (_type == X) ? XTICKS : YTICKS);
	es = fm.tightBoundingRect(QString::number(l.max));
	ss = fm.tightBoundingRect(QString::number(l.min));
	ls = fm.tightBoundingRect(_label);

	if (_type == X) {
		_boundingRect = QRectF(-ss.width()/2, -TICK/2,
		_size + es.width()/2 + ss.width()/2,
		ls.height() + es.height() - fm.descent() + TICK + 2*PADDING + 1);
	} else {
		int mtw = 0;
		QRect ts;
		qreal val;

		for (int i = 0; i < ((l.max - l.min) / l.d) + 1; i++) {
			val = l.min + i * l.d;
			QString str = QString::number(val);
			ts = fm.tightBoundingRect(str);
			mtw = qMax(ts.width(), mtw);
		}

		_boundingRect = QRectF(-(ls.height() + mtw + 2*PADDING + TICK/2),
		  -(_size + es.height()/2 + fm.descent()), ls.height() + mtw + 2*PADDING
		  + TICK, _size + es.height()/2 + fm.descent() + ss.height()/2);
	}
}

void AxisItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ts, ls;
	struct Label l;
	qreal range = _range.size();
	qreal val;
	QPen pen = QPen(Qt::black, AXIS_WIDTH);


	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setFont(font);
	painter->setPen(pen);

	ls = fm.tightBoundingRect(_label);

	if (_type == X) {
		painter->drawLine(0, 0, _size, 0);

		l = label(_range.min(), _range.max(), XTICKS);
		for (int i = 0; i < ((l.max - l.min) / l.d) + 1; i++) {
			val = l.min + i * l.d;
			QString str = QString::number(val);

			painter->drawLine((_size/range) * (val - _range.min()), TICK/2,
			  (_size/range) * (val - _range.min()), -TICK/2);
			ts = fm.tightBoundingRect(str);
			painter->drawText(((_size/range) * (val - _range.min()))
			  - (ts.width()/2), ts.height() + TICK/2 + PADDING, str);
		}

		painter->drawText(_size/2 - ls.width()/2, ls.height() + ts.height()
		  - 2*fm.descent() + TICK/2 + 2*PADDING, _label);
	} else {
		painter->drawLine(0, 0, 0, -_size);

		l = label(_range.min(), _range.max(), YTICKS);
		int mtw = 0;
		for (int i = 0; i < ((l.max - l.min) / l.d) + 1; i++) {
			val = l.min + i * l.d;
			QString str = QString::number(val);

			painter->drawLine(TICK/2, -((_size/range) * (val - _range.min())),
			  -TICK/2, -((_size/range) * (val - _range.min())));
			ts = fm.tightBoundingRect(str);
			mtw = qMax(ts.width(), mtw);
			painter->drawText(-(ts.width() + PADDING + TICK/2), -((_size/range)
			  * (val - _range.min())) + (ts.height()/2), str);
		}

		painter->rotate(-90);
		painter->drawText(_size/2 - ls.width()/2, -(mtw + 2*PADDING + TICK/2),
		  _label);
		painter->rotate(90);
	}

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

QSizeF AxisItem::margin() const
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	QFontMetrics fm(font);
	QRect ss, es, ls;
	struct Label l;


	l = label(_range.min(), _range.max(), (_type == X) ? XTICKS : YTICKS);
	es = fm.tightBoundingRect(QString::number(l.max));
	ss = fm.tightBoundingRect(QString::number(l.min));
	ls = fm.tightBoundingRect(_label);

	if (_type == X) {
		return QSizeF(es.width()/2,
		  ls.height() + es.height() - fm.descent() + TICK/2 + 2*PADDING);
	} else {
		int mtw = 0;
		QRect ts;
		qreal val;

		for (int i = 0; i < ((l.max - l.min) / l.d) + 1; i++) {
			val = l.min + i * l.d;
			QString str = QString::number(val);
			ts = fm.tightBoundingRect(str);
			mtw = qMax(ts.width(), mtw);
		}

		return QSizeF(ls.height() -fm.descent() + mtw + 2*PADDING
		  + TICK/2, es.height()/2 + fm.descent());
	}
}

QList<qreal> AxisItem::ticks() const
{
	struct Label l;
	QList<qreal> list;

	l = label(_range.min(), _range.max(), (_type == X) ? XTICKS : YTICKS);
	for (int i = 0; i < ((l.max - l.min) / l.d) + 1; i++)
		list.append(((_size/_range.size()) * ((l.min + i * l.d)
		  - _range.min())));

	return list;
}
