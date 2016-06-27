#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPaintEngine>
#include <QPaintDevice>
#include "config.h"
#include "axisitem.h"
#include "slideritem.h"
#include "sliderinfoitem.h"
#include "infoitem.h"
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

	connect(_slider, SIGNAL(positionChanged(const QPointF&)), this,
	  SLOT(emitSliderPositionChanged(const QPointF&)));
	connect(_scene, SIGNAL(mouseClicked(const QPointF&)), this,
	  SLOT(newSliderPosition(const QPointF&)));

	_info = new InfoItem();

	_sliderInfo = new SliderInfoItem(_slider);
	_sliderInfo->setZValue(2.0);

	_xScale = 1;
	_yScale = 1;
	_yOffset = 0;

	_precision = 0;
	_minYRange = 0.01;

	_sliderPos = 0;
}

GraphView::~GraphView()
{
	if (_xAxis->scene() != _scene)
		delete _xAxis;
	if (_yAxis->scene() != _scene)
		delete _yAxis;

	if (_slider->scene() != _scene)
		delete _slider;

	if (_info->scene() != _scene)
		delete _info;
}

void GraphView::updateBounds(const QPointF &point)
{
	if (point.x() < _bounds.left())
		_bounds.setLeft(point.x());
	if (point.x() > _bounds.right())
		_bounds.setRight(point.x());
	if (point.y() > _bounds.bottom())
		_bounds.setBottom(point.y());
	if (point.y() < _bounds.top())
		_bounds.setTop(point.y());
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

void GraphView::loadData(const QVector<QPointF> &data)
{
	QPainterPath path;
	QGraphicsPathItem *pi;


	if (data.size() < 2)
		return;

	if (!_graphs.size())
		_bounds.moveTo(data.at(0));

	updateBounds(data.at(0));
	path.moveTo(data.at(0).x(), -data.at(0).y());
	for (int i = 1; i < data.size(); i++) {
		const QPointF &p = data.at(i);
		path.lineTo(p.x(), -p.y());
		updateBounds(p);
	}

	pi = new QGraphicsPathItem(path);
	QBrush brush(_palette.color(), Qt::SolidPattern);
	QPen pen(brush, 0);
	pi->setPen(pen);
	_scene->addItem(pi);
	_graphs.append(pi);
}

void GraphView::redraw()
{
	if (!_graphs.isEmpty())
		redraw(viewport()->size() - QSizeF(MARGIN, MARGIN));
}

void GraphView::redraw(const QSizeF &size)
{
	QRectF r;
	QSizeF mx, my;
	RangeF rx, ry;
	QTransform transform;
	qreal xs, ys;


	if (_xAxis->scene() == _scene)
		_scene->removeItem(_xAxis);
	if (_yAxis->scene() == _scene)
		_scene->removeItem(_yAxis);
	if (_slider->scene() == _scene)
		_scene->removeItem(_slider);
	if (_info->scene() == _scene)
		_scene->removeItem(_info);

	for (int i = 0; i < _graphs.size(); i++)
		_graphs.at(i)->resetTransform();

	rx = RangeF(_bounds.left() * _xScale, _bounds.right() * _xScale);
	ry = RangeF(_bounds.top() * _yScale + _yOffset, _bounds.bottom() * _yScale
	  + _yOffset);
	if (ry.size() < _minYRange)
		ry.resize(_minYRange);

	_xAxis->setRange(rx);
	_yAxis->setRange(ry);
	mx = _xAxis->margin();
	my = _yAxis->margin();

	r = _scene->itemsBoundingRect();
	if (r.height() < _minYRange)
		r.adjust(0, -(_minYRange/2 - r.height()/2), 0,
		  _minYRange/2 - r.height()/2);

	xs = (size.width() - (my.width() + mx.width())) / r.width();
	ys = (size.height() - (mx.height() + my.height())
	  - _info->boundingRect().height()) / r.height();
	transform.scale(xs, ys);

	for (int i = 0; i < _graphs.size(); i++)
		_graphs.at(i)->setTransform(transform);

	r = _scene->itemsBoundingRect();
	if (r.height() < _minYRange * ys)
		r.adjust(0, -(_minYRange/2 * ys - r.height()/2), 0,
		  (_minYRange/2) * ys - r.height()/2);

	_xAxis->setSize(r.width());
	_yAxis->setSize(r.height());
	_xAxis->setPos(r.bottomLeft());
	_yAxis->setPos(r.bottomLeft());
	_scene->addItem(_xAxis);
	_scene->addItem(_yAxis);

	_slider->setArea(r);
	if (_sliderPos > _bounds.right() || _sliderPos < _bounds.left())
		_slider->setVisible(false);
	_slider->setPos((_sliderPos / _bounds.width()) * _slider->area().width(),
	  r.bottom());
	_scene->addItem(_slider);

	updateSliderInfo();

	r = _scene->itemsBoundingRect();
	_info->setPos(r.topLeft() + QPointF(r.width()/2
	  - _info->boundingRect().width()/2, -_info->boundingRect().height()));
	_scene->addItem(_info);

	_scene->setSceneRect(_scene->itemsBoundingRect());
}

void GraphView::resizeEvent(QResizeEvent *)
{
	if (!_graphs.empty())
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
	if (_xAxis->scene() == _scene)
		_scene->removeItem(_xAxis);
	if (_yAxis->scene() == _scene)
		_scene->removeItem(_yAxis);
	if (_slider->scene() == _scene)
		_scene->removeItem(_slider);
	if (_info->scene() == _scene)
		_scene->removeItem(_info);

	_slider->clear();
	_info->clear();
	_scene->clear();
	_graphs.clear();
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

void GraphView::updateSliderInfo()
{
	_sliderInfo->setVisible(_graphs.size() == 1);

	if (!_sliderInfo->isVisible())
		return;

	const QPainterPath &path = _graphs.at(0)->path();
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
	if (_graphs.isEmpty())
		return;

	_sliderPos = (pos.x() / _slider->area().width()) * _bounds.width();
	emit sliderPositionChanged(_sliderPos);

	updateSliderInfo();
}

void GraphView::setSliderPosition(qreal pos)
{
	_sliderPos = pos;

	if (_graphs.isEmpty())
		return;

	if (pos > _bounds.right() || pos < _bounds.left())
		_slider->setVisible(false);
	else {
		_slider->setPos((pos / _bounds.width()) * _slider->area().width(), 0);
		_slider->setVisible(true);
	}
}

void GraphView::newSliderPosition(const QPointF &pos)
{
	if (_slider->area().contains(pos)) {
		_slider->setPos(pos);
		_slider->setVisible(true);
		emitSliderPositionChanged(pos);
	}
}

void GraphView::addInfo(const QString &key, const QString &value)
{
	_info->insert(key, value);
}

void GraphView::clearInfo()
{
	_info->clear();
}
