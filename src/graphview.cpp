#include <float.h>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QGraphicsSimpleTextItem>
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

	_xMax = -FLT_MAX;
	_xMin = FLT_MAX;
	_yMax = -FLT_MAX;
	_yMin = FLT_MAX;

	_xScale = 1;
	_yScale = 1;

	_precision = 0;
	_minRange = 0.01;
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

	delete _scene;
}

void GraphView::updateBounds(const QPointF &point)
{
	if (point.x() < _xMin)
		_xMin = point.x();
	if (point.x() > _xMax)
		_xMax = point.x();
	if (point.y() < _yMin)
		_yMin = point.y();
	if (point.y() > _yMax)
		_yMax = point.y();
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

void GraphView::setXScale(qreal scale)
{
	_xScale = scale;
}

void GraphView::setYScale(qreal scale)
{
	_yScale = scale;
}

void GraphView::loadData(const QVector<QPointF> &data)
{
	QPainterPath path;
	QGraphicsPathItem *pi;


	if (data.size() < 2)
		return;

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

	if (_graphs.size() > 1)
		_sliderInfo->hide();

	redraw();
}

void GraphView::redraw()
{
	if (!_graphs.isEmpty())
		resize(viewport()->size() - QSizeF(MARGIN, MARGIN));
}

void GraphView::resize(const QSizeF &size)
{
	QRectF r;
	QSizeF mx, my;
	QPointF rx, ry;
	QTransform transform;
	qreal xs, ys, diff;


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

	rx = QPointF(_xMin * _xScale, _xMax * _xScale);
	ry = QPointF(_yMin * _yScale, _yMax * _yScale);
	if ((diff = ry.y() - ry.x()) < _minRange)
		ry = QPointF(ry.x() - (_minRange/2 - diff/2),
		  ry.y() + (_minRange/2 - diff/2));
	_xAxis->setRange(rx);
	_yAxis->setRange(ry);
	mx = _xAxis->margin();
	my = _yAxis->margin();

	r = _scene->itemsBoundingRect();
	if (r.height() < _minRange)
		r.adjust(0, -(_minRange/2 - r.height()/2), 0,
		  _minRange/2 - r.height()/2);

	xs = (size.width() - (my.width() + mx.width())) / r.width();
	ys = (size.height() - (mx.height() + my.height())
	  - _info->boundingRect().height()) / r.height();
	transform.scale(xs, ys);

	for (int i = 0; i < _graphs.size(); i++)
		_graphs.at(i)->setTransform(transform);

	r = _scene->itemsBoundingRect();
	if (r.height() < _minRange * ys)
		r.adjust(0, -(_minRange/2 * ys - r.height()/2), 0,
		  (_minRange/2) * ys - r.height()/2);

	_xAxis->setSize(r.width());
	_yAxis->setSize(r.height());
	_xAxis->setPos(r.bottomLeft());
	_yAxis->setPos(r.bottomLeft());
	_scene->addItem(_xAxis);
	_scene->addItem(_yAxis);

	qreal sp = (_slider->pos().x() == _slider->area().left())
		? 0 : (_slider->pos().x() - _slider->area().left())
		  / _slider->area().width();
	_slider->setArea(r);
	_slider->setPos(QPointF(sp * r.width(), r.bottom()));
	_scene->addItem(_slider);

	const QPainterPath &path = _graphs.at(0)->path();
	QPointF p = path.pointAtPercent(sp);
	_sliderInfo->setText(QString::number(-p.y() * _yScale, 'f', _precision));

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
	qreal ratio = target.width() / target.height();
	QSizeF orig = _scene->sceneRect().size();
	QSizeF canvas = QSizeF(orig.height() * ratio, orig.height());

	resize(canvas);
	_slider->hide();
	_info->hide();
	_scene->render(painter, target, QRectF());
	_slider->show();
	_info->show();
	resize(orig);
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
	_sliderInfo->show();

	_slider->clear();
	_info->clear();
	_scene->clear();
	_graphs.clear();
	_palette.reset();

	_xMax = -FLT_MAX;
	_xMin = FLT_MAX;
	_yMax = -FLT_MAX;
	_yMin = FLT_MAX;

	_scene->setSceneRect(0, 0, 0, 0);
}

void GraphView::emitSliderPositionChanged(const QPointF &pos)
{
	if (_graphs.isEmpty())
		return;

	qreal val = pos.x() / _slider->area().width();
	emit sliderPositionChanged(val * (_xMax - _xMin));

	const QPainterPath &path = _graphs.at(0)->path();
	QRectF br = path.boundingRect();
	if (br.height() < _minRange)
		br.adjust(0, -(_minRange/2 - br.height()/2), 0,
		  _minRange/2 - br.height()/2);
	QPointF p = path.pointAtPercent(val);
	qreal r = (p.y() - br.bottom()) / br.height();
	_sliderInfo->setPos(QPointF(0, _slider->boundingRect().height() * r));
	_sliderInfo->setText(QString::number(-p.y() * _yScale, 'f', _precision));
}

qreal GraphView::sliderPosition() const
{
	if (!_slider->isVisible())
		return -1;
	else
		return (_slider->pos().x() / _slider->area().width()) * (_xMax - _xMin);
}

void GraphView::setSliderPosition(qreal pos)
{
	if (pos > (_xMax - _xMin))
		_slider->setVisible(false);
	else {
		_slider->setPos((pos / (_xMax - _xMin)) * _slider->area().width(), 0);
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
