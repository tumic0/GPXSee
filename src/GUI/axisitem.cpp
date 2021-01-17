#include <cmath>
#include <QPainter>
#include "common/util.h"
#include "font.h"
#include "axisitem.h"


#define AXIS_WIDTH  1
#define TICK        6
#define PADDING     6
#define XTICKS      15
#define YTICKS      10

class Ticks
{
public:
	Ticks(double minValue, double maxValue, int maxCount);

	int count() const {return ((int)((_max - _min) / _d)) + 1;}
	double val(int i) const {return _min + i * _d;}
	double min() const {return _min;}
	double max() const {return _max;}

private:
	double _min;
	double _max;
	double _d;
};

Ticks::Ticks(double minValue, double maxValue, int maxCount)
{
	double range = Util::niceNum(maxValue - minValue, false);
	_d = Util::niceNum(range / maxCount, false);
	_min = ceil(minValue / _d) * _d;
	_max = floor(maxValue / _d) * _d;
}


AxisItem::AxisItem(Type type, QGraphicsItem *parent)
  : QGraphicsItem(parent), _locale(QLocale::system())
{
	_type = type;
	_size = 0;
	_zoom = 1.0;

	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);
}

void AxisItem::setRange(const RangeF &range)
{
	prepareGeometryChange();
	_range = range;

	QFontMetrics fm(_font);
	Ticks ticks(_range.min(), _range.max(),
	  (_type == X) ? XTICKS * _zoom : YTICKS * _zoom);
	_ticks = QVector<Tick>(ticks.count());

	for (int i = 0; i < ticks.count(); i++) {
		Tick &t = _ticks[i];
		t.value = ticks.val(i);
		t.boundingBox = fm.tightBoundingRect(_locale.toString(t.value));
	}

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

void AxisItem::updateBoundingRect()
{
	QFontMetrics fm(_font);
	QRect es = _ticks.isEmpty() ? QRect() : _ticks.last().boundingBox;
	QRect ss = _ticks.isEmpty() ? QRect() : _ticks.first().boundingBox;

	if (_type == X) {
		_boundingRect = QRectF(-ss.width()/2, -TICK/2, _size + es.width()/2
		  + ss.width()/2, es.height() - 2*fm.descent() + TICK + 2*PADDING);
	} else {
		int mtw = 0;
		for (int i = 0; i < _ticks.count(); i++)
			mtw = qMax(_ticks.at(i).boundingBox.width(), mtw);
		_boundingRect = QRectF(-(mtw + 2*PADDING + TICK/2 - fm.descent()),
		  -(_size + es.height()/2 + fm.descent()), mtw + 2*PADDING
		  + TICK - fm.descent(), _size + es.height()/2 + fm.descent()
		  + ss.height()/2);
	}
}

void AxisItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QRect ts;

	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setFont(_font);
	QPen pen(painter->pen());
	pen.setWidth(AXIS_WIDTH);
	painter->setPen(pen);

	if (_type == X) {
		painter->drawLine(0, 0, _size, 0);

		for (int i = 0; i < _ticks.count(); i++) {
			qreal val = _ticks.at(i).value;
			ts = _ticks.at(i).boundingBox;

			painter->drawLine((_size/_range.size()) * (val - _range.min()),
			  TICK/2, (_size/_range.size()) * (val - _range.min()), -TICK/2);
			painter->drawText(((_size/_range.size()) * (val - _range.min()))
			  - (ts.width()/2), ts.height() + TICK/2 + PADDING,
			  _locale.toString(val));
		}
	} else {
		painter->drawLine(0, 0, 0, -_size);

		int mtw = 0;
		for (int i = 0; i < _ticks.count(); i++) {
			qreal val = _ticks.at(i).value;
			ts = _ticks.at(i).boundingBox;
			mtw = qMax(ts.width(), mtw);

			painter->drawLine(TICK/2, -((_size/_range.size())
			  * (val - _range.min())), -TICK/2, -((_size/_range.size())
			  * (val - _range.min())));
			painter->drawText(-(ts.width() + PADDING + TICK/2),
			  -((_size/_range.size()) * (val - _range.min())) + (ts.height()/2),
			  _locale.toString(val));
		}
	}

	//painter->setPen(Qt::red);
	//painter->drawRect(boundingRect());
}

QSizeF AxisItem::margin() const
{
	QFontMetrics fm(_font);
	QRect es = _ticks.isEmpty() ? QRect() : _ticks.last().boundingBox;

	if (_type == X) {
		return QSizeF(es.width()/2, es.height() - 2*fm.descent() + TICK/2
		  + 2*PADDING);
	} else {
		int mtw = 0;
		for (int i = 0; i < _ticks.count(); i++)
			mtw = qMax(_ticks.at(i).boundingBox.width(), mtw);

		return QSizeF(mtw + 2*PADDING + TICK/2 - fm.descent(),
		  es.height()/2 + fm.descent());
	}
}

QList<qreal> AxisItem::ticks() const
{
	QList<qreal> list;

	for (int i = 0; i < _ticks.count(); i++)
		list.append(((_size/_range.size()) * (_ticks.at(i).value
		  - _range.min())));

	return list;
}
