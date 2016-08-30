#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPaintEngine>
#include <QPaintDevice>
#include "config.h"
#include "axisitem.h"
#include "slideritem.h"
#include "sliderinfoitem.h"
#include "infoitem.h"
#include "graphitem.h"
#include "graphview.h"


#define MARGIN 10.0


void Scene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
		emit mouseClicked(e->scenePos());

	QGraphicsScene::mousePressEvent(e);
}


GraphView::GraphView(QWidget *parent)
	: QGraphicsView(parent)
{
	_scene = new Scene(this);
	setScene(_scene);

	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	_xAxis = new AxisItem(AxisItem::X);
	_yAxis = new AxisItem(AxisItem::Y);
	_slider = new SliderItem();
	_slider->setZValue(2.0);
	_sliderInfo = new SliderInfoItem(_slider);
	_sliderInfo->setZValue(2.0);
	_info = new InfoItem();

	connect(_slider, SIGNAL(positionChanged(const QPointF&)), this,
	  SLOT(emitSliderPositionChanged(const QPointF&)));
	connect(_scene, SIGNAL(mouseClicked(const QPointF&)), this,
	  SLOT(newSliderPosition(const QPointF&)));

	_xScale = 1;
	_yScale = 1;
	_yOffset = 0;

	_precision = 0;
	_minYRange = 0.01;

	_sliderPos = 0;
}

void GraphView::createXLabel()
{
	_xAxis->setLabel(QString("%1 [%2]").arg(_xLabel).arg(_xUnits));
}

void GraphView::createYLabel()
{
	_yAxis->setLabel(QString("%1 [%2]").arg(_yLabel).arg(_yUnits));
}

void GraphView::setXLabel(const QString &label)
{
	_xLabel = label;
	createXLabel();
}

void GraphView::setYLabel(const QString &label)
{
	_yLabel = label;
	createYLabel();
}

void GraphView::setXUnits(const QString &units)
{
	_xUnits = units;
	createXLabel();
}

void GraphView::setYUnits(const QString &units)
{
	_yUnits = units;
	createYLabel();
}

void GraphView::loadData(const QVector<QPointF> &data, int id)
{
	QPainterPath path;
	GraphItem *pi;


	if (data.size() < 2)
		return;

	path.moveTo(data.at(0).x(), -data.at(0).y());
	for (int i = 1; i < data.size(); i++) {
		const QPointF &p = data.at(i);
		path.lineTo(p.x(), -p.y());
	}

	pi = new GraphItem(path);
	pi->setId(id);
	pi->setColor(_palette.color());

	_graphs.append(pi);

	if (!_hide.contains(id)) {
		_visible.append(pi);
		_scene->addItem(pi);
		updateBounds(path);
	}
}

void GraphView::removeItem(QGraphicsItem *item)
{
	if (item->scene() == _scene)
		_scene->removeItem(item);
}

void GraphView::addItem(QGraphicsItem *item)
{
	if (item->scene() != _scene)
		_scene->addItem(item);
}

void GraphView::showGraph(bool show, int id)
{
	if (show)
		_hide.remove(id);
	else
		_hide.insert(id);

	_visible.clear();
	_bounds = QRectF();
	for (int i = 0; i < _graphs.count(); i++) {
		GraphItem* gi = _graphs.at(i);
		if (_hide.contains(gi->id()))
			removeItem(gi);
		else {
			addItem(gi);
			_visible.append(gi);
			updateBounds(gi->path());
		}
	}
}

void GraphView::redraw()
{
	redraw(viewport()->size() - QSizeF(MARGIN, MARGIN));
}

void GraphView::updateBounds(const QPainterPath &path)
{
	QRectF br = path.boundingRect();
	br.moveTopLeft(QPointF(br.left(), -br.top() - br.height()));
	_bounds |= br;
}

QRectF GraphView::graphsBoundingRect() const
{
	QRectF rect;

	for (int i = 0; i < _visible.count(); i++)
		rect |= _visible.at(i)->boundingRect();

	return rect;
}

void GraphView::redraw(const QSizeF &size)
{
	QRectF r;
	QSizeF mx, my;
	RangeF rx, ry;
	QTransform transform;
	qreal xs, ys;


	if (_visible.isEmpty()) {
		removeItem(_xAxis);
		removeItem(_yAxis);
		removeItem(_slider);
		removeItem(_info);
		_scene->setSceneRect(QRectF());
		return;
	}

	addItem(_xAxis);
	addItem(_yAxis);
	addItem(_slider);
	addItem(_info);

	rx = RangeF(_bounds.left() * _xScale, _bounds.right() * _xScale);
	ry = RangeF(_bounds.top() * _yScale + _yOffset, _bounds.bottom() * _yScale
	  + _yOffset);
	if (ry.size() < _minYRange)
		ry.resize(_minYRange);

	_xAxis->setRange(rx);
	_yAxis->setRange(ry);
	mx = _xAxis->margin();
	my = _yAxis->margin();

	r = graphsBoundingRect();
	if (r.height() < _minYRange)
		r.adjust(0, -(_minYRange/2 - r.height()/2), 0,
		  _minYRange/2 - r.height()/2);

	xs = (size.width() - (my.width() + mx.width())) / r.width();
	ys = (size.height() - (mx.height() + my.height())
	  - _info->boundingRect().height()) / r.height();
	transform.scale(xs, ys);

	for (int i = 0; i < _visible.size(); i++)
		_visible.at(i)->setTransform(transform);

	QPointF p(r.left() * xs, r.top() * ys);
	QSizeF s(r.width() * xs, r.height() * ys);
	r = QRectF(p, s);
	if (r.height() < _minYRange * ys)
		r.adjust(0, -(_minYRange/2 * ys - r.height()/2), 0,
		  (_minYRange/2) * ys - r.height()/2);

	_xAxis->setSize(r.width());
	_yAxis->setSize(r.height());
	_xAxis->setPos(r.bottomLeft());
	_yAxis->setPos(r.bottomLeft());

	_slider->setArea(r);
	updateSliderPosition();

	r |= _xAxis->sceneBoundingRect();
	r |= _yAxis->sceneBoundingRect();
	_info->setPos(r.topLeft() + QPointF(r.width()/2
	  - _info->boundingRect().width()/2, -_info->boundingRect().height()));

	_scene->setSceneRect(_scene->itemsBoundingRect());
}

void GraphView::resizeEvent(QResizeEvent *)
{
	redraw();
}

void GraphView::plot(QPainter *painter, const QRectF &target)
{
	qreal ratio = painter->paintEngine()->paintDevice()->logicalDpiX()
	  / SCREEN_DPI;
	QSizeF canvas = QSizeF(target.width() / ratio, target.height() / ratio);

	setUpdatesEnabled(false);
	redraw(canvas);
	if (_slider->pos().x() == _slider->area().left())
		_slider->hide();
	_scene->render(painter, target);
	_slider->show();
	redraw();
	setUpdatesEnabled(true);
}

void GraphView::clear()
{
	_slider->clear();
	_info->clear();

	for (int i = 0; i < _graphs.count(); i++)
		delete _graphs[i];

	_graphs.clear();
	_visible.clear();
	_palette.reset();

	_bounds = QRectF();
	_sliderPos = 0;

	_scene->setSceneRect(0, 0, 0, 0);
}

static qreal yAtX(const QPainterPath &path, qreal x)
{
	int low = 0;
	int high = path.elementCount() - 1;
	int mid = 0;

	Q_ASSERT(high > low);
	Q_ASSERT(x >= path.elementAt(low).x && x <= path.elementAt(high).x);

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const QPainterPath::Element &e = path.elementAt(mid);
		if (e.x > x)
			high = mid - 1;
		else if (e.x < x)
			low = mid + 1;
		else
			return e.y;
	}

	QLineF l;
	if (path.elementAt(mid).x < x)
		l = QLineF(path.elementAt(mid).x, path.elementAt(mid).y,
		  path.elementAt(mid+1).x, path.elementAt(mid+1).y);
	else
		l = QLineF(path.elementAt(mid-1).x, path.elementAt(mid-1).y,
		  path.elementAt(mid).x, path.elementAt(mid).y);

	return l.pointAt((x - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

void GraphView::updateSliderPosition()
{
	if (_bounds.width() <= 0)
		return;

	if (_sliderPos <= _bounds.right() && _sliderPos >= _bounds.left()) {
		_slider->setPos((_sliderPos / _bounds.width()) * _slider->area().width(),
		  _slider->area().bottom());
		_slider->setVisible(!_visible.isEmpty());
	} else {
		_slider->setPos(_slider->area().left(), _slider->area().bottom());
		_slider->setVisible(false);
	}

	updateSliderInfo();
}

void GraphView::updateSliderInfo()
{
	_sliderInfo->setVisible(_visible.count() == 1);
	if (!_sliderInfo->isVisible())
		return;

	const QPainterPath &path = _visible.first()->path();
	QRectF br = path.boundingRect();
	if (br.height() < _minYRange)
		br.adjust(0, -(_minYRange/2 - br.height()/2), 0,
		  _minYRange/2 - br.height()/2);

	qreal y = yAtX(path, _sliderPos);
	qreal r = (y - br.bottom()) / br.height();

	qreal pos = (_sliderPos / _bounds.width()) * _slider->area().width();
	SliderInfoItem::Side s = (pos + _sliderInfo->boundingRect().width()
	  > _slider->area().right()) ? SliderInfoItem::Left : SliderInfoItem::Right;

	_sliderInfo->setSide(s);
	_sliderInfo->setPos(QPointF(0, _slider->boundingRect().height() * r));
	_sliderInfo->setText(QString::number(-y * _yScale + _yOffset, 'f',
	  _precision));
}

void GraphView::emitSliderPositionChanged(const QPointF &pos)
{
	if (_slider->area().width() <= 0)
		return;

	_sliderPos = (pos.x() / _slider->area().width()) * _bounds.width();
	updateSliderPosition();

	emit sliderPositionChanged(_sliderPos);
}

void GraphView::setSliderPosition(qreal pos)
{
	if (_visible.isEmpty())
		return;

	_sliderPos = pos;
	updateSliderPosition();
}

void GraphView::newSliderPosition(const QPointF &pos)
{
	if (_slider->area().contains(pos))
		_slider->setPos(pos);
}

void GraphView::addInfo(const QString &key, const QString &value)
{
	_info->insert(key, value);
}

void GraphView::clearInfo()
{
	_info->clear();
}
